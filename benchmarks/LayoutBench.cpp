// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


#include "aether/LayoutEngine.h"
#include <benchmark/benchmark.h>
#include <random>

namespace jaether {

static void BM_ComponentCreation(benchmark::State& state) {
    for (auto _ : state) {
        JComponentStorage storage;
        JComponentHandle parent;
        
        for (int i = 0; i < state.range(0); ++i) {
            auto child = storage.createComponent(JComponentType::Button, parent);
            if (i == 0) parent = child;
        }
    }
}

BENCHMARK(BM_ComponentCreation)
    ->Range(10, 10000);

static void BM_PropertySet(benchmark::State& state) {
    JComponentStorage storage;
    auto handle = storage.createComponent(JComponentType::Button);
    
    for (auto _ : state) {
        storage.getComponent(handle)->properties.setProperty(
            JPropertyId::Width, JPropertyValue(100));
    }
}

BENCHMARK(BM_PropertySet);

static void BM_IncrementalLayout(benchmark::State& state) {
    JComponentStorage storage;
    JLayoutEngine engine(storage);
    
    auto root = storage.createComponent(JComponentType::Container);
    storage.getComponent(root)->layoutResult = {0, 0, 800, 600};
    
    for (int i = 0; i < state.range(0); ++i) {
        storage.createComponent(JComponentType::Button, root);
    }
    
    engine.forceRelayout();
    
    for (auto _ : state) {
        auto& entries = const_cast<std::vector<JComponentEntry>&>(
            reinterpret_cast<const JComponentStorage&>(storage).size() > 0 ? 
            *reinterpret_cast<std::vector<JComponentEntry>*>(1) : 
            *reinterpret_cast<std::vector<JComponentEntry>*>(1));
        
        auto handles = storage.getRoot();
        if (handles.isValid()) {
            engine.markDirty(handles, JPropertyId::Width);
            engine.relayoutIfNeeded();
        }
    }
}

BENCHMARK(BM_IncrementalLayout)
    ->Range(100, 1000);

static void BM_FlexboxLayout(benchmark::State& state) {
    for (auto _ : state) {
        JComponentStorage storage;
        JLayoutEngine engine(storage);
        
        auto root = storage.createComponent(JComponentType::Container);
        auto* rootEntry = storage.getComponent(root);
        rootEntry->layoutResult = {0, 0, 800, 600};
        rootEntry->properties.setProperty(JPropertyId::JFlexDirection, JPropertyValue(JFlexDirection::Row));
        
        std::vector<JComponentHandle> children;
        for (int i = 0; i < state.range(0); ++i) {
            auto child = storage.createComponent(JComponentType::Button, root);
            auto* childEntry = storage.getComponent(child);
            childEntry->properties.setProperty(JPropertyId::Width, JPropertyValue(100));
            childEntry->properties.setProperty(JPropertyId::Height, JPropertyValue(50));
            children.push_back(child);
        }
        
        engine.forceRelayout();
    }
}

BENCHMARK(BM_FlexboxLayout)
    ->Range(10, 1000);

static void BM_NestedFlexboxLayout(benchmark::State& state) {
    for (auto _ : state) {
        JComponentStorage storage;
        JLayoutEngine engine(storage);
        
        auto root = storage.createComponent(JComponentType::Container);
        auto* rootEntry = storage.getComponent(root);
        rootEntry->layoutResult = {0, 0, 800, 600};
        rootEntry->properties.setProperty(JPropertyId::JFlexDirection, JPropertyValue(JFlexDirection::Row));
        
        for (int row = 0; row < state.range(0); ++row) {
            auto rowContainer = storage.createComponent(JComponentType::Container, root);
            auto* rowEntry = storage.getComponent(rowContainer);
            rowEntry->properties.setProperty(JPropertyId::JFlexDirection, JPropertyValue(JFlexDirection::Row));
            
            for (int col = 0; col < 5; ++col) {
                auto button = storage.createComponent(JComponentType::Button, rowContainer);
                auto* buttonEntry = storage.getComponent(button);
                buttonEntry->properties.setProperty(JPropertyId::Width, JPropertyValue(100));
                buttonEntry->properties.setProperty(JPropertyId::Height, JPropertyValue(50));
            }
        }
        
        engine.forceRelayout();
    }
}

BENCHMARK(BM_NestedFlexboxLayout)
    ->Range(2, 50);

}
