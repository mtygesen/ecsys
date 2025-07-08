#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <chrono>
#include <vector>

#include "registry.hpp"

namespace ecsys::benchmark {
class Timer {
   public:
    Timer() : _start(std::chrono::high_resolution_clock::now()) {}

    void reset() { _start = std::chrono::high_resolution_clock::now(); }

    double elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - _start).count();
    }

   private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
};

template <class T>
struct TestComponent {
    T a;
};

void RunBenchmark(const size_t entityCount) {
    volatile size_t sink = 0;

    Timer timer;
    Registry registry;

    std::vector<EntityId> entities;
    entities.resize(entityCount);

    timer.reset();
    for (size_t i = 0; i < entityCount; ++i) {
        entities[i] = registry.create();
    }

    double elapsed = timer.elapsed();
    std::cout << "Entity creation time for " << entityCount << " entities: " << elapsed << "ms\n";

    timer.reset();
    for (size_t i = 0; i < entityCount; ++i) {
        registry.emplace<TestComponent<int>>(entities[i], 1);
    }

    double elasped = timer.elapsed();
    std::cout << "Component addition time for " << entityCount << " entities: " << elasped
              << "ms\n";

    timer.reset();
    for (size_t i = 0; i < entityCount; ++i) {
        auto &comp = registry.get<TestComponent<int>>(entities[i]);
        sink += static_cast<size_t>(comp.a);
    }

    elapsed = timer.elapsed();
    std::cout << "Component retrieval time for " << entityCount << " entities: " << elapsed
              << "ms\n";

    timer.reset();
    for (size_t i = 0; i < entityCount; ++i) {
        registry.remove<TestComponent<int>>(entities[i]);
    }

    elapsed = timer.elapsed();

    std::cout << "Component removal time for " << entityCount << " entities: " << elapsed << "ms\n";

    timer.reset();
    for (size_t i = 0; i < entityCount; ++i) {
        registry.destroy(entities[i]);
    }

    elapsed = timer.elapsed();
    std::cout << "Entity destruction time for " << entityCount << " entities: " << elapsed
              << "ms\n";

    for (size_t i = 0; i < entityCount; ++i) {
        entities[i] = registry.create();
    }

    for (size_t i = 0; i < entityCount; ++i) {
        registry.emplace<TestComponent<int>>(entities[i], 1);
        registry.emplace<TestComponent<double>>(entities[i], 2.0);
    }

    auto view = registry.view<TestComponent<int>, TestComponent<double>>();
    timer.reset();
    view.forEach([&sink](TestComponent<int> &comp1, TestComponent<double> &comp2) {
        sink += static_cast<size_t>(comp1.a) + static_cast<size_t>(comp2.a);
    });

    elapsed = timer.elapsed();
    std::cout << "View iteration time for " << entityCount << " entities: " << elapsed << "ms\n";
}
}  // namespace ecsys::benchmark

#endif