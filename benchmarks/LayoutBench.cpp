#include "aether/LayoutEngine.h"
#include <benchmark/benchmark.h>
#include <random>

namespace aether {

static void BM_ComponentCreation(benchmark::State& state) {
    for (auto _ : state) {
        ComponentStorage storage;
        ComponentHandle parent;
        
        for (int i = 0; i < state.range(0); ++i) {
            auto child = storage.createComponent(ComponentType::Button, parent);
            if (i == 0) parent = child;
        }
    }
}

BENCHMARK(BM_ComponentCreation)
    ->Range(10, 10000);

static void BM_PropertySet(benchmark::State& state) {
    ComponentStorage storage;
    auto handle = storage.createComponent(ComponentType::Button);
    
    for (auto _ : state) {
        storage.getComponent(handle)->properties.setProperty(
            PropertyId::Width, PropertyValue(100));
    }
}

BENCHMARK(BM_PropertySet);

static void BM_IncrementalLayout(benchmark::State& state) {
    ComponentStorage storage;
    LayoutEngine engine(storage);
    
    auto root = storage.createComponent(ComponentType::Container);
    storage.getComponent(root)->layoutResult = {0, 0, 800, 600};
    
    for (int i = 0; i < state.range(0); ++i) {
        storage.createComponent(ComponentType::Button, root);
    }
    
    engine.forceRelayout();
    
    for (auto _ : state) {
        auto& entries = const_cast<std::vector<ComponentEntry>&>(
            reinterpret_cast<const ComponentStorage&>(storage).size() > 0 ? 
            *reinterpret_cast<std::vector<ComponentEntry>*>(1) : 
            *reinterpret_cast<std::vector<ComponentEntry>*>(1));
        
        auto handles = storage.getRoot();
        if (handles.isValid()) {
            engine.markDirty(handles, PropertyId::Width);
            engine.relayoutIfNeeded();
        }
    }
}

BENCHMARK(BM_IncrementalLayout)
    ->Range(100, 1000);

static void BM_FlexboxLayout(benchmark::State& state) {
    for (auto _ : state) {
        ComponentStorage storage;
        LayoutEngine engine(storage);
        
        auto root = storage.createComponent(ComponentType::Container);
        auto* rootEntry = storage.getComponent(root);
        rootEntry->layoutResult = {0, 0, 800, 600};
        rootEntry->properties.setProperty(PropertyId::FlexDirection, PropertyValue(FlexDirection::Row));
        
        std::vector<ComponentHandle> children;
        for (int i = 0; i < state.range(0); ++i) {
            auto child = storage.createComponent(ComponentType::Button, root);
            auto* childEntry = storage.getComponent(child);
            childEntry->properties.setProperty(PropertyId::Width, PropertyValue(100));
            childEntry->properties.setProperty(PropertyId::Height, PropertyValue(50));
            children.push_back(child);
        }
        
        engine.forceRelayout();
    }
}

BENCHMARK(BM_FlexboxLayout)
    ->Range(10, 1000);

static void BM_NestedFlexboxLayout(benchmark::State& state) {
    for (auto _ : state) {
        ComponentStorage storage;
        LayoutEngine engine(storage);
        
        auto root = storage.createComponent(ComponentType::Container);
        auto* rootEntry = storage.getComponent(root);
        rootEntry->layoutResult = {0, 0, 800, 600};
        rootEntry->properties.setProperty(PropertyId::FlexDirection, PropertyValue(FlexDirection::Row));
        
        for (int row = 0; row < state.range(0); ++row) {
            auto rowContainer = storage.createComponent(ComponentType::Container, root);
            auto* rowEntry = storage.getComponent(rowContainer);
            rowEntry->properties.setProperty(PropertyId::FlexDirection, PropertyValue(FlexDirection::Row));
            
            for (int col = 0; col < 5; ++col) {
                auto button = storage.createComponent(ComponentType::Button, rowContainer);
                auto* buttonEntry = storage.getComponent(button);
                buttonEntry->properties.setProperty(PropertyId::Width, PropertyValue(100));
                buttonEntry->properties.setProperty(PropertyId::Height, PropertyValue(50));
            }
        }
        
        engine.forceRelayout();
    }
}

BENCHMARK(BM_NestedFlexboxLayout)
    ->Range(2, 50);

}
