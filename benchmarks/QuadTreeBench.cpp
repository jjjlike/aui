#include "aether/QuadTree.h"
#include <benchmark/benchmark.h>
#include <random>

namespace aether {

static void BM_QuadTreeRebuild(benchmark::State& state) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);
    
    std::vector<ComponentHandle> components;
    std::vector<Rect> bounds;
    
    for (int i = 0; i < state.range(0); ++i) {
        components.push_back(ComponentHandle{i, 1});
        bounds.push_back({dist(rng), dist(rng), 30.0f, 30.0f});
    }
    
    for (auto _ : state) {
        QuadTree tree(Rect{0, 0, 1000, 1000});
        tree.rebuild(components, [&bounds](ComponentHandle h) {
            return bounds[h.index];
        });
    }
}

BENCHMARK(BM_QuadTreeRebuild)
    ->Range(100, 10000);

static void BM_QuadTreeQuery(benchmark::State& state) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);
    
    std::vector<ComponentHandle> components;
    std::vector<Rect> bounds;
    
    for (int i = 0; i < state.range(0); ++i) {
        components.push_back(ComponentHandle{i, 1});
        bounds.push_back({dist(rng), dist(rng), 30.0f, 30.0f});
    }
    
    QuadTree tree(Rect{0, 0, 1000, 1000});
    tree.rebuild(components, [&bounds](ComponentHandle h) {
        return bounds[h.index];
    });
    
    std::vector<Point> queryPoints;
    for (int i = 0; i < 1000; ++i) {
        queryPoints.push_back({dist(rng), dist(rng)});
    }
    
    int queryIndex = 0;
    for (auto _ : state) {
        auto result = tree.query(queryPoints[queryIndex++ % queryPoints.size()]);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_QuadTreeQuery)
    ->Range(100, 10000);

static void BM_QuadTreeUpdate(benchmark::State& state) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);
    
    std::vector<ComponentHandle> components;
    std::vector<Rect> bounds;
    
    for (int i = 0; i < state.range(0); ++i) {
        components.push_back(ComponentHandle{i, 1});
        bounds.push_back({dist(rng), dist(rng), 30.0f, 30.0f});
    }
    
    QuadTree tree(Rect{0, 0, 1000, 1000});
    tree.rebuild(components, [&bounds](ComponentHandle h) {
        return bounds[h.index];
    });
    
    for (auto _ : state) {
        int idx = rng() % state.range(0);
        Rect newBounds{dist(rng), dist(rng), 30.0f, 30.0f};
        tree.update(components[idx], newBounds);
        bounds[idx] = newBounds;
    }
}

BENCHMARK(BM_QuadTreeUpdate)
    ->Range(100, 10000);

static void BM_QuadTreeManyUpdates(benchmark::State& state) {
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);
    
    std::vector<ComponentHandle> components;
    std::vector<Rect> bounds;
    
    for (int i = 0; i < 1000; ++i) {
        components.push_back(ComponentHandle{i, 1});
        bounds.push_back({dist(rng), dist(rng), 30.0f, 30.0f});
    }
    
    QuadTree tree(Rect{0, 0, 1000, 1000});
    tree.rebuild(components, [&bounds](ComponentHandle h) {
        return bounds[h.index];
    });
    
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            int idx = rng() % components.size();
            Rect newBounds{dist(rng), dist(rng), 30.0f, 30.0f};
            tree.update(components[idx], newBounds);
            bounds[idx] = newBounds;
        }
    }
}

BENCHMARK(BM_QuadTreeManyUpdates)
    ->Range(10, 100);

}
