#ifndef SPARSE_SET_HPP
#define SPARSE_SET_HPP

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "util/types.hpp"

namespace ecsys {
class ISparseSet {
   public:
    virtual ~ISparseSet() = default;
    virtual void remove(EntityId id) = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual void clear() = 0;
    virtual bool contains(EntityId id) const = 0;
    virtual std::vector<EntityId> getEntities() const = 0;
};

template <class Obj>
class SparseSet : public ISparseSet {
   public:
    Obj *add(EntityId id) { return add(id, Obj{}); }

    Obj *add(EntityId id, Obj &&obj) {
        size_t denseIdx = getDenseIndex(id);
        if (denseIdx != TOMBSTONE) {
            _denseLayer[denseIdx] = std::move(obj);
            _denseToEntity[denseIdx] = id;
            return &_denseLayer[denseIdx];
        }

        setDenseIndex(id, _denseLayer.size());

        _denseLayer.push_back(std::move(obj));
        _denseToEntity.push_back(id);
        return &_denseLayer.back();
    }

    template <class... Args>
    Obj *emplace(EntityId id, Args &&...args) {
        size_t denseIdx = getDenseIndex(id);
        if (denseIdx != TOMBSTONE) {
            _denseLayer[denseIdx] = Obj(std::forward<Args>(args)...);
            _denseToEntity[denseIdx] = id;
            return &_denseLayer[denseIdx];
        }

        setDenseIndex(id, _denseLayer.size());
        _denseLayer.emplace_back(std::forward<Args>(args)...);
        _denseToEntity.push_back(id);
        return &_denseLayer.back();
    }

    Obj *get(EntityId id) {
        size_t denseIdx = getDenseIndex(id);
        return (denseIdx != TOMBSTONE) ? &_denseLayer[denseIdx] : nullptr;
    }

    Obj &getRef(EntityId id) {
        size_t denseIdx = getDenseIndex(id);
        if (denseIdx == TOMBSTONE) {
            assert(false && "Entity does not exist in SparseSet");
        }

        return _denseLayer[denseIdx];
    }

    const std::vector<Obj> &data() const { return _denseLayer; }

    void remove(EntityId id) override {
        size_t denseIdx = getDenseIndex(id);
        if (denseIdx == TOMBSTONE) {
            return;
        }

        setDenseIndex(_denseToEntity.back(), denseIdx);
        setDenseIndex(id, TOMBSTONE);

        std::swap(_denseLayer.back(), _denseLayer[denseIdx]);
        std::swap(_denseToEntity.back(), _denseToEntity[denseIdx]);

        _denseLayer.pop_back();
        _denseToEntity.pop_back();
    }

    size_t size() const override { return _denseLayer.size(); }

    bool empty() const override { return _denseLayer.empty(); }

    void clear() override {
        _sparseLayer.clear();
        _denseLayer.clear();
        _denseToEntity.clear();
    }

    bool contains(EntityId id) const override { return getDenseIndex(id) != TOMBSTONE; }

    std::vector<EntityId> getEntities() const { return _denseToEntity; }

   private:
    size_t getDenseIndex(EntityId id) const {
        size_t pageNumber = id / MAX_PAGE_SIZE;
        size_t pageIdx = id % MAX_PAGE_SIZE;
        if (pageNumber < _sparseLayer.size() && pageIdx < _sparseLayer[pageNumber].size()) {
            return _sparseLayer[pageNumber][pageIdx];
        }

        return TOMBSTONE;
    }

    void setDenseIndex(EntityId id, size_t idx) {
        size_t pageNumber = id / MAX_PAGE_SIZE;
        size_t pageIdx = id % MAX_PAGE_SIZE;
        if (pageNumber >= _sparseLayer.size()) {
            _sparseLayer.resize(pageNumber + 1);
        }

        if (pageIdx >= _sparseLayer[pageNumber].size()) {
            _sparseLayer[pageNumber].resize(MAX_PAGE_SIZE, TOMBSTONE);
        }

        _sparseLayer[pageNumber][pageIdx] = idx;
    }

    static constexpr size_t TOMBSTONE = std::numeric_limits<size_t>::max();
    static constexpr size_t MAX_PAGE_SIZE = 2048;
    using Page = std::vector<size_t>;

    std::vector<Page> _sparseLayer;
    std::vector<Obj> _denseLayer;
    std::vector<EntityId> _denseToEntity;
};
}  // namespace ecsys

#endif