#include "registry.hpp"

using namespace ecsys;

EntityId Registry::createEntity() {
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

void Registry::destroyEntity(EntityId &id) {
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

Registry::ComponentMask &Registry::getEntityMask(EntityId id) {
    ComponentMask *mask = _entityMasks.get(id);
    assert(mask != nullptr && "Entity mask not found");
    return *mask;
}
