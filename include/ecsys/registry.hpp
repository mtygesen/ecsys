#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <array>
#include <bitset>
#include <cassert>
#include <functional>
#include <memory>
#include <numeric>

#include "ecsys/container/sparse_set.hpp"
#include "ecsys/util/concepts.hpp"
#include "ecsys/util/types.hpp"

namespace ecsys {
template <class... Components>
    requires UniqueTypes<Components...>
class View;
class Registry {
   public:
    EntityId create();
    void destroy(EntityId &id);
    void clear();
    size_t getPoolCount() const;
    size_t getEntityCount() const;
    void printEntityComponents(EntityId id);

    template <class Obj>
    Obj *add(EntityId id) {
        return add(id, Obj{});
    }

    template <class Obj>
    Obj *add(EntityId id, Obj &&comp) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");

        SparseSet<Obj> &pool = getComponentPool<Obj>();
        if (pool.get(id)) {
            return pool.add(id, std::forward<Obj>(comp));
        }

        auto &mask = getEntityMask(id);
        setComponentBit<Obj>(mask, true);
        return pool.add(id, std::forward<Obj>(comp));
    }

    template <class Obj, class... Args>
    Obj *emplace(EntityId id, Args &&...args) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");

        SparseSet<Obj> &pool = getComponentPool<Obj>();
        if (pool.get(id)) {
            return pool.emplace(id, std::forward<Args>(args)...);
        }

        auto &mask = getEntityMask(id);
        setComponentBit<Obj>(mask, true);
        return pool.emplace(id, std::forward<Args>(args)...);
    }

    template <class Obj>
    Obj &get(EntityId id) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");
        SparseSet<Obj> &pool = getComponentPool<Obj>();
        Obj *comp = pool.get(id);
        assert(comp != nullptr && "Component not found for entity");
        return *comp;
    }

    template <class Obj>
    void remove(EntityId id) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");
        SparseSet<Obj> &pool = getComponentPool<Obj>();
        if (!pool.get(id)) {
            return;
        }

        auto &mask = getEntityMask(id);
        setComponentBit<Obj>(mask, false);

        pool.remove(id);
    }

    template <class... Components>
        requires UniqueTypes<Components...>
    bool hasAll(EntityId id) {
        auto &mask = getEntityMask(id);
        return (getComponentBit<Components>(mask) && ...);
    }

    template <class... Components>
        requires UniqueTypes<Components...>
    bool hasAny(EntityId id) {
        auto &mask = getEntityMask(id);
        return (getComponentBit<Components>(mask) || ...);
    }

    template <class... Components>
        requires UniqueTypes<Components...>
    View<Components...> view() {
        return {this};
    }

   private:
    template <class... Components>
        requires UniqueTypes<Components...>
    friend class View;

    static constexpr size_t MAX_COMPONENTS = 64;
    using ComponentMask = std::bitset<MAX_COMPONENTS>;

    template <class Obj>
    void setComponentBit(ComponentMask &mask, bool value) {
        size_t idx = getOrRegisterComponentIndex<Obj>();
        mask[idx] = value;
    }

    template <class Obj>
    bool getComponentBit(ComponentMask &mask) {
        size_t idx = getOrRegisterComponentIndex<Obj>();
        return mask[idx];
    }

    template <class Obj>
    SparseSet<Obj> &getComponentPool() {
        ISparseSet *genericPool = getComponentPoolPtr<Obj>();
        SparseSet<Obj> *pool = static_cast<SparseSet<Obj> *>(genericPool);
        return *pool;
    }

    template <class Obj>
    ISparseSet *getComponentPoolPtr() {
        size_t idx = getOrRegisterComponentIndex<Obj>();
        return _componentPools[idx].get();
    }

    template <class Obj>
    size_t getOrRegisterComponentIndex() {
        size_t idx = getComponentIndex<Obj>();
        if (idx >= _componentPools.size() || !_componentPools[idx]) {
            registerComponent<Obj>();
        }

        return idx;
    }

    template <class Obj>
    void registerComponent() {
        assert(_componentPools.size() < MAX_COMPONENTS && "Maximum number of components reached");
        size_t idx = getComponentIndex<Obj>();
        if (idx >= _componentPools.size()) {
            _componentPools.resize(idx + 1);
        }

        _componentPools[idx] = std::make_unique<SparseSet<Obj>>();
    }

    template <class Obj>
    static size_t getComponentIndex() {
        static size_t idx = getNextComponentIndex(typeid(Obj).name());
        return idx;
    }

    static size_t getNextComponentIndex(const std::string &typeName) {
        static size_t nextIndex = 0;
        _componentNames.push_back(typeName);
        return nextIndex++;
    }

    ComponentMask &getEntityMask(EntityId id) {
        auto *mask = _entityMasks.get(id);
        assert(mask != nullptr && "Entity mask not found");
        return *mask;
    }

    std::vector<EntityId> _availableEntities;

    SparseSet<ComponentMask> _entityMasks;

    std::vector<std::unique_ptr<ISparseSet>> _componentPools;

    inline static std::vector<std::string> _componentNames;

    static constexpr size_t MAX_ENTITIES = 10000000;
    static constexpr size_t NULL_ENTITY = std::numeric_limits<EntityId>::max();
    EntityId _entityCounter = 0;
};

template <class... Components>
    requires UniqueTypes<Components...>
class View {
   public:
    View(Registry *registry)
        : _registry(registry), _viewPools{_registry->getComponentPoolPtr<Components>()...} {
        auto smallestPool =
            std::min_element(_viewPools.begin(), _viewPools.end(),
                             [](ISparseSet *a, ISparseSet *b) { return a->size() < b->size(); });
        assert(smallestPool != _viewPools.end() && "No component pools found");

        _smallestPool = *smallestPool;
    }

    void forEach(std::function<void(Components &...)> func) { forEachImpl(func); }

    void forEach(std::function<void(EntityId, Components &...)> func) { forEachImpl(func); }

   private:
    template <class Func>
    void forEachImpl(Func func) {
        constexpr auto idxs = std::make_index_sequence<sizeof...(Components)>{};
        for (EntityId id : _smallestPool->getEntities()) {
            if (allContain(id)) {
                if constexpr (std::is_invocable_v<Func, EntityId, Components &...>) {
                    std::apply(func,
                               std::tuple_cat(std::make_tuple(id), makeComponentTuple(id, idxs)));
                } else if constexpr (std::is_invocable_v<Func, Components &...>) {
                    std::apply(func, makeComponentTuple(id, idxs));
                } else {
                    static_assert(false,
                                  "Function signature must be either void(EntityId, Components "
                                  "&...) or void(Components &...)");
                }
            }
        }
    }

    bool allContain(EntityId id) {
        return std::all_of(_viewPools.begin(), _viewPools.end(),
                           [id](ISparseSet *pool) { return pool->contains(id); });
    }

    template <size_t... Idxs>
    auto makeComponentTuple(EntityId id, std::index_sequence<Idxs...> /*unused*/) {
        return std::make_tuple(std::ref(getPoolAt<Idxs>()->getRef(id))...);
    }

    template <class... Types>
    struct typeList {
        using typeTuple = std::tuple<Components...>;

        template <size_t Idx>
        using get = std::tuple_element_t<Idx, typeTuple>;

        static constexpr size_t size = sizeof...(Components);
    };

    using componentTypes = typeList<Components...>;

    template <size_t Idx>
    auto getPoolAt() {
        using ComponentType = typename componentTypes::template get<Idx>;
        return static_cast<SparseSet<ComponentType> *>(_viewPools[Idx]);
    }

    Registry *_registry;
    std::array<ISparseSet *, sizeof...(Components)> _viewPools;
    ISparseSet *_smallestPool = nullptr;
};
}  // namespace ecsys

#endif