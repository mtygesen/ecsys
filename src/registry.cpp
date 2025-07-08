#include "registry.hpp"

#include <iostream>
#include <sstream>

using namespace ecsys;

EntityId Registry::create() {
    EntityId id = NULL_ENTITY;
    if (_availableEntities.empty()) {
        assert(_entityCounter < MAX_ENTITIES && "Maximum number of entities reached");
        id = _entityCounter++;
    } else {
        id = _availableEntities.back();
        _availableEntities.pop_back();
    }

    assert(id != NULL_ENTITY && "Entity id cannot be NULL_ENTITY");
    _entityMasks.add(id);

    return id;
}

void Registry::destroy(EntityId &id) {
    ComponentMask &mask = getEntityMask(id);
    for (size_t i = 0; i < mask.size(); ++i) {
        if (mask[i]) {
            _componentPools[i]->remove(id);
        }
    }

    _entityMasks.remove(id);
    _availableEntities.push_back(id);
    id = NULL_ENTITY;
}

void Registry::clear() {
    _availableEntities.clear();
    _entityMasks.clear();
    _componentPools.clear();
    _entityCounter = 0;
}

size_t Registry::getPoolCount() const { return _componentPools.size(); }

size_t Registry::getEntityCount() const { return _entityMasks.size(); }

void Registry::printEntityComponents(EntityId id) {
    assert(id != NULL_ENTITY && "Cannot print components of NULL_ENTITY");
    assert(id < MAX_ENTITIES && "Entity id out of bounds");

    std::stringstream ss;
    std::string prefix;
    auto entityNumber = id + 1;
    ss << "Entity " << entityNumber << " has components: ";
    auto &mask = getEntityMask(id);
    bool found = false;
    for (size_t i = 0; i < mask.size(); ++i) {
        if (mask[i]) {
            ss << prefix << _componentNames[i];
            prefix = ", ";
            found = true;
        }
    }

    if (!found) {
        std::cout << "Entity " << entityNumber << " has no components\n";
        return;
    }

    std::cout << ss.str() << '\n';
}

Registry::ComponentMask &Registry::getEntityMask(EntityId id) {
    auto *mask = _entityMasks.get(id);
    assert(mask != nullptr && "Entity mask not found");
    return *mask;
}
