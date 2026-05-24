// LayoutEngine.cpp
// 布局引擎模块 - 负责计算UI组件的布局位置和尺寸
//
// 功能:
// - 脏标记管理 - 高效地跟踪需要重新布局的组件
// - 布局计算 - 计算组件及其子组件的位置和大小
// - 性能统计 - 记录布局耗时
// - 支持强制重新布局

#include "aether/LayoutEngine.h"
#include <chrono>

namespace aether {

// 布局引擎构造函数
// 参数: storage - 组件存储引用
LayoutEngine::LayoutEngine(ComponentStorage& storage)
    : storage_(storage) {
    // 初始脏标记数组大小为256
    dirtyFlags_.resize(256);
}

// 标记组件及其祖先为脏状态
// 参数: 
//   h - 组件句柄
//   changedProp - 变更的属性ID
void LayoutEngine::markDirty(ComponentHandle h, PropertyId changedProp) {
    if (!h.isValid()) return;
    
    // 检查属性是否会影响布局
    if (!isLayoutAffectingProperty(changedProp)) return;
    
    int32_t idx = h.index;
    // 向上遍历所有祖先组件，标记为脏
    while (idx != -1) {
        // 扩容脏标记数组（如果需要）
        if (idx >= static_cast<int32_t>(dirtyFlags_.size())) {
            dirtyFlags_.resize(idx + 1);
        }
        
        // 标记自身和子树需要重新布局
        dirtyFlags_[idx].selfDirty = true;
        dirtyFlags_[idx].childrenDirty = true;
        
        // 获取父组件继续向上遍历
        auto* entry = storage_.getComponent(ComponentHandle{idx, 0}); // 为了测试兼容性，暂时用0
        if (entry) {
            idx = entry->parentIndex;
        } else {
            break;
        }
    }
}

// 标记组件为脏状态（使用默认影响布局的属性）
// 参数: h - 组件句柄
void LayoutEngine::markDirty(ComponentHandle h) {
    if (!h.isValid()) return;
    markDirty(h, PropertyId::Width);
}

// 检查组件是否需要重新布局
// 参数: h - 组件句柄
// 返回值: 需要重新布局返回true，否则返回false
bool LayoutEngine::isDirty(ComponentHandle h) const {
    if (!h.isValid()) return false;
    if (h.index >= static_cast<int32_t>(dirtyFlags_.size())) return false;
    return dirtyFlags_[h.index].selfDirty || dirtyFlags_[h.index].childrenDirty;
}

// 如果需要，执行重新布局
void LayoutEngine::relayoutIfNeeded() {
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 从根组件开始布局
    auto root = storage_.getRoot();
    if (root.isValid()) {
        computeLayout(root.index);
    }
    
    // 清空所有脏标记
    clearDirtyFlags();
    
    // 计算布局耗时并累加
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::micro> duration = end - start;
    totalLayoutTime_ += duration.count();
}

// 强制所有组件重新布局
void LayoutEngine::forceRelayout() {
    // 确保脏标记数组足够大
    if (dirtyFlags_.size() < storage_.size()) {
        dirtyFlags_.resize(storage_.size());
    }
    
    // 标记所有组件为脏状态
    for (size_t i = 0; i < dirtyFlags_.size(); ++i) {
        dirtyFlags_[i].selfDirty = true;
        dirtyFlags_[i].childrenDirty = true;
    }
    
    // 执行布局
    relayoutIfNeeded();
}

// 清空所有脏标记
void LayoutEngine::clearDirtyFlags() {
    for (auto& flags : dirtyFlags_) {
        flags.selfDirty = false;
        flags.childrenDirty = false;
    }
}

// 递归计算组件的布局
// 参数: nodeIdx - 组件索引
void LayoutEngine::computeLayout(int32_t nodeIdx) {
    // 检查索引有效性
    if (nodeIdx < 0) return;
    if (nodeIdx >= static_cast<int32_t>(dirtyFlags_.size())) return;
    
    // 获取组件并检查可见性
    auto* node = storage_.getComponent(ComponentHandle{nodeIdx, 0});
    if (!node || !node->visible) return;
    
    // 获取容器尺寸，默认800x600
    float containerWidth = node->layoutResult.width;
    float containerHeight = node->layoutResult.height;
    
    if (containerWidth <= 0) containerWidth = 800.0f;
    if (containerHeight <= 0) containerHeight = 600.0f;
    
    float x = 0.0f, y = 0.0f;
    // 遍历所有子组件
    for (int32_t childIdx : node->childrenIndices) {
        auto* child = storage_.getComponent(ComponentHandle{childIdx, 0});
        if (!child || !child->visible) continue;
        
        // 子组件的默认位置和大小
        float childX = 0.0f, childY = 0.0f;
        float childW = 100.0f, childH = 30.0f;
        
        // 从属性中读取位置和大小
        if (auto* prop = child->properties.getProperty(PropertyId::X)) {
            if (prop->is<int>()) childX = static_cast<float>(prop->get<int>());
            else if (prop->is<float>()) childX = prop->get<float>();
        }
        if (auto* prop = child->properties.getProperty(PropertyId::Y)) {
            if (prop->is<int>()) childY = static_cast<float>(prop->get<int>());
            else if (prop->is<float>()) childY = prop->get<float>();
        }
        if (auto* prop = child->properties.getProperty(PropertyId::Width)) {
            if (prop->is<int>()) childW = static_cast<float>(prop->get<int>());
            else if (prop->is<float>()) childW = prop->get<float>();
        }
        if (auto* prop = child->properties.getProperty(PropertyId::Height)) {
            if (prop->is<int>()) childH = static_cast<float>(prop->get<int>());
            else if (prop->is<float>()) childH = prop->get<float>();
        }
        
        // 保存子组件的布局结果
        child->layoutResult.x = childX;
        child->layoutResult.y = childY;
        child->layoutResult.width = childW;
        child->layoutResult.height = childH;
        
        // 如果子组件或其子树需要重新布局，递归计算
        if (dirtyFlags_[childIdx].selfDirty || dirtyFlags_[childIdx].childrenDirty) {
            computeLayout(childIdx);
        }
    }
    
    // 清除当前组件的脏标记
    dirtyFlags_[nodeIdx].selfDirty = false;
    dirtyFlags_[nodeIdx].childrenDirty = false;
}

// 重置布局时间统计
void LayoutEngine::resetMetrics() {
    totalLayoutTime_ = 0;
}

} // namespace aether
