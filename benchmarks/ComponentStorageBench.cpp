#include "aether/ComponentStorage.h"
#include <benchmark/benchmark.h>
#include <random>

namespace jaether {

static void BM_ComponentStorageCreate(benchmark::State& state) {
    for (auto _ : state) {
        JComponentStorage storage;
        
        for (int i = 0; i < state.range(0); ++i) {
            storage.createComponent(JComponentType::Button);
        }
    }
}

BENCHMARK(BM_ComponentStorageCreate)
    ->Range(100, 100000);

static void BM_ComponentStorageCreateWithParent(benchmark::State& state) {
    for (auto _ : state) {
        JComponentStorage storage;
        
        auto root = storage.createComponent(JComponentType::Container);
        
        for (int i = 0; i < state.range(0); ++i) {
            storage.createComponent(JComponentType::Button, root);
        }
    }
}

BENCHMARK(BM_ComponentStorageCreateWithParent)
    ->Range(10, 10000);

static void BM_ComponentStorageDestroy(benchmark::State& state) {
    for (auto _ : state) {
        JComponentStorage storage;
        std::vector<JComponentHandle> handles;
        
        for (int i = 0; i < state.range(0); ++i) {
            handles.push_back(storage.createComponent(JComponentType::Button));
        }
        
        for (auto& h : handles) {
            storage.destroyComponent(h);
        }
    }
}

BENCHMARK(BM_ComponentStorageDestroy)
    ->Range(100, 10000);

static void BM_ComponentStorageIteration(benchmark::State& state) {
    JComponentStorage storage;
    
    for (int i = 0; i < state.range(0); ++i) {
        storage.createComponent(JComponentType::Button);
    }
    
    for (auto _ : state) {
        int count = 0;
        storage.forEach([&count](JComponentHandle) {
            count++;
        });
        benchmark::DoNotOptimize(count);
    }
}

BENCHMARK(BM_ComponentStorageIteration)
    ->Range(100, 100000);

static void BM_ComponentStoragePropertyAccess(benchmark::State& state) {
    JComponentStorage storage;
    auto handle = storage.createComponent(JComponentType::Button);
    auto* entry = storage.getComponent(handle);
    
    for (int i = 0; i < 100; ++i) {
        entry->properties.setProperty(
            static_cast<JPropertyId>(i + 1), JPropertyValue(i));
    }
    
    for (auto _ : state) {
        for (int i = 0; i < 100; ++i) {
            auto* value = entry->properties.getProperty(static_cast<JPropertyId>(i + 1));
            benchmark::DoNotOptimize(value);
        }
    }
}

BENCHMARK(BM_ComponentStoragePropertyAccess);

static void BM_ComponentStorageHandleValidation(benchmark::State& state) {
    JComponentStorage storage;
    std::vector<JComponentHandle> handles;
    
    for (int i = 0; i < state.range(0); ++i) {
        handles.push_back(storage.createComponent(JComponentType::Button));
    }
    
    for (auto _ : state) {
        bool valid = true;
        for (const auto& h : handles) {
            valid = valid && storage.isValid(h);
        }
        benchmark::DoNotOptimize(valid);
    }
}

BENCHMARK(BM_ComponentStorageHandleValidation)
    ->Range(100, 10000);

static void BM_ComponentStorageFindById(benchmark::State& state) {
    JComponentStorage storage;
    std::vector<JComponentId> ids;
    
    for (int i = 0; i < state.range(0); ++i) {
        auto h = storage.createComponent(JComponentType::Button);
        auto* entry = storage.getComponent(h);
        ids.push_back(entry->id);
    }
    
    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(0, ids.size() - 1);
    
    for (auto _ : state) {
        auto handle = storage.findById(ids[dist(rng)]);
        benchmark::DoNotOptimize(handle);
    }
}

BENCHMARK(BM_ComponentStorageFindById)
    ->Range(100, 10000);

}
