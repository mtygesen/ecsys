#ifndef SPARSE_SET_HPP
#define SPARSE_SET_HPP

#include <algorithm>
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
    virtual void printDense() const = 0;
};

template <class Component>
class SparseSet : public ISparseSet {
   public:
    Component *add(EntityId id) { return add(id, Component{}); }

    Component *add(EntityId id, Component &&comp) {
        size_t denseIdx = getDenseIndex(id);
        if (denseIdx != TOMBSTONE) {
            _denseLayer[denseIdx] = std::move(comp);
            _denseToEntity[denseIdx] = id;
            return &_denseLayer[denseIdx];
        }

        setDenseIndex(id, _denseLayer.size());

        _denseLayer.push_back(std::move(comp));
        _denseToEntity.push_back(id);
        return &_denseLayer.back();
    }

    Component *get(EntityId id) {
        size_t denseIdx = getDenseIndex(id);
        return (denseIdx != TOMBSTONE) ? &_denseLayer[denseIdx] : nullptr;
    }

    const std::vector<Component> &data() const { return _denseLayer; }

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

    void printDense() const override {
        std::stringstream ss;
        std::string delim;
        for (const Component &comp : _denseLayer) {
            ss << delim << comp;
            if (delim.empty()) {
                delim = ", ";
            }
        }

        std::cout << ss.str() << '\n';
    }

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
    std::vector<Component> _denseLayer;
    std::vector<EntityId> _denseToEntity;
};
}  // namespace ecsys

#endif