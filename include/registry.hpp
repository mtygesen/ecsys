#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <bitset>
#include <cassert>
#include <memory>
#include <numeric>

#include "container/sparse_set.hpp"
#include "util/types.hpp"

namespace ecsys {
class Registry {
   public:
    EntityId createEntity();
    void destroyEntity(EntityId &id);
    void clear();

    template <class Component>
    Component *addComponent(EntityId id, Component &&comp) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && id >= 0 && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");

        SparseSet<Component> &pool = getComponentPool<Component>();
        if (pool.get(id)) {
            return pool.add(id, std::forward<Component>(comp));
        }

        ComponentMask &mask = getEntityMask(id);
        setComponentBit<Component>(mask, true);
        return pool.add(id, std::forward<Component>(comp));
    }

    template <class Component>
    Component &getComponent(EntityId id) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && id >= 0 && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");
        SparseSet<Component> &pool = getComponentPool<Component>();
        Component *comp = pool.get(id);
        assert(comp != nullptr && "Component not found for entity");
        return *comp;
    }

    template <class Component>
    void removeComponent(EntityId id) {
        assert(id != NULL_ENTITY && "Cannot add component to NULL_ENTITY");
        assert(id < MAX_ENTITIES && "Entity id out of bounds");
        assert(_entityMasks.get(id) != nullptr && "Entity mask not found");
        SparseSet<Component> &pool = getComponentPool<Component>();
        if (!pool.get(id)) {
            return;
        }

        ComponentMask &mask = getEntityMask(id);
        setComponentBit<Component>(mask, false);

        pool.remove(id);
    }

    template <class... Components>
    bool hasComponents(EntityId id) {
        auto &mask = getEntityMask(id);
        return (getComponentBit<Components>(mask) && ...);
    }

    template <class... Components>
    bool hasAnyComponent(EntityId id) {
        auto &mask = getEntityMask(id);
        return (getComponentBit<Components>(mask) || ...);
    }

   private:
    static constexpr size_t MAX_COMPONENTS = 64;
    using ComponentMask = std::bitset<MAX_COMPONENTS>;

    template <class Component>
    void setComponentBit(ComponentMask &mask, bool value) {
        size_t idx = getOrRegisterComponentIndex<Component>();
        mask[idx] = value;
    }

    template <class Component>
    bool getComponentBit(ComponentMask &mask) {
        size_t idx = getOrRegisterComponentIndex<Component>();
        return mask[idx];
    }

    template <class Component>
    SparseSet<Component> &getComponentPool() {
        ISparseSet *genericPool = getComponentPoolPtr<Component>();
        SparseSet<Component> *pool = static_cast<SparseSet<Component> *>(genericPool);
        return *pool;
    }

    template <class Component>
    ISparseSet *getComponentPoolPtr() {
        size_t idx = getOrRegisterComponentIndex<Component>();
        return _componentPools[idx].get();
    }

    template <class Component>
    size_t getOrRegisterComponentIndex() {
        size_t idx = getComponentIndex<Component>();
        if (idx >= _componentPools.size() || !_componentPools[idx]) {
            registerComponent<Component>();
        }

        return idx;
    }

    template <class Component>
    void registerComponent() {
        assert(_componentPools.size() < MAX_COMPONENTS && "Maximum number of components reached");
        size_t idx = getComponentIndex<Component>();
        if (idx >= _componentPools.size()) {
            _componentPools.resize(idx + 1);
        }

        _componentPools[idx] = std::make_unique<SparseSet<Component>>();
    }

    template <class Component>
    static size_t getComponentIndex() {
        static size_t idx = getNextComponentIndex();
        return idx;
    }

    static size_t getNextComponentIndex() {
        static size_t nextIndex = 0;
        return nextIndex++;
    }

    ComponentMask &getEntityMask(EntityId id);

    std::vector<EntityId> _availableEntities;

    SparseSet<ComponentMask> _entityMasks;

    std::vector<std::unique_ptr<ISparseSet>> _componentPools;

    static constexpr size_t MAX_ENTITIES = 1000000;
    static constexpr size_t NULL_ENTITY = std::numeric_limits<EntityId>::max();
    EntityId _entityCounter = 0;
};
}  // namespace ecsys

#endif