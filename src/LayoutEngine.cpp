// LayoutEngine.cpp
// 布局引擎模块 - 负责计算UI组件的布局位置和尺寸
//
// 功能:
// - 脏标记管理 - 高效地跟踪需要重新布局的组件
// - 布局计算 - 计算组件及其子组件的位置和大小（使用Flexbox布局算法）
// - 性能统计 - 记录布局耗时
// - 支持强制重新布局
//
// Flexbox布局算法实现说明:
// 1. 首先测量所有子组件的基准尺寸
// 2. 计算可用空间和总flexGrow、flexShrink
// 3. 根据flexGrow分配额外空间
// 4. 根据flexShrink分配不足空间
// 5. 计算最终尺寸并确定位置

#include "aether/LayoutEngine.h"
#include "aether/Logger.h"
#include <algorithm>
#include <cmath>
#include <chrono>

namespace aether {

// 布局引擎构造函数
// 参数: storage - 组件存储引用
LayoutEngine::LayoutEngine(ComponentStorage& storage)
    : storage_(storage) {
    // 初始脏标记数组大小为256
    dirtyFlags_.resize(256);
}

// 设置布局引擎模式
// 参数: mode - 要设置的布局模式
void LayoutEngine::setMode(LayoutEngineMode mode) {
    mode_ = mode;
}

// 获取当前布局引擎模式
// 返回值: 当前布局模式
LayoutEngineMode LayoutEngine::getMode() const {
    return mode_;
}

// 标记组件及其祖先为脏状态
// 参数:
//   h - 组件句柄
//   changedProp - 变更的属性ID
void LayoutEngine::markDirty(ComponentHandle h, PropertyId changedProp) {
    if (!storage_.isValid(h)) return;
    
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
        
        // 直接通过索引访问（因为LayoutEngine是ComponentStorage的友元）
        if (idx >= 0 && idx < static_cast<int32_t>(storage_.entries_.size())) {
            idx = storage_.entries_[idx].parentIndex;
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

// 获取属性值的浮点数值
// 参数:
//   entry - 组件条目引用
//   propId - 属性ID
//   defaultValue - 默认值
// 返回值: 属性的浮点数值
static float getPropertyFloat(ComponentEntry& entry, PropertyId propId, float defaultValue) {
    if (auto* prop = entry.properties.getProperty(propId)) {
        if (prop->is<int>()) return static_cast<float>(prop->get<int>());
        if (prop->is<float>()) return prop->get<float>();
    }
    return defaultValue;
}

// 获取属性值的整型数值
// 参数:
//   entry - 组件条目引用
//   propId - 属性ID
//   defaultValue - 默认值
// 返回值: 属性的整型数值
static int getPropertyInt(ComponentEntry& entry, PropertyId propId, int defaultValue) {
    if (auto* prop = entry.properties.getProperty(propId)) {
        if (prop->is<int>()) return prop->get<int>();
        if (prop->is<float>()) return static_cast<int>(prop->get<float>());
        if (prop->is<FlexDirection>()) return static_cast<int>(prop->get<FlexDirection>());
        if (prop->is<FlexWrap>()) return static_cast<int>(prop->get<FlexWrap>());
        if (prop->is<JustifyContent>()) return static_cast<int>(prop->get<JustifyContent>());
        if (prop->is<AlignItems>()) return static_cast<int>(prop->get<AlignItems>());
        if (prop->is<AlignContent>()) return static_cast<int>(prop->get<AlignContent>());
    }
    return defaultValue;
}

// 获取组件的外边距
// 参数:
//   entry - 组件条目引用
//   margin - 输出参数，外边距数组[left, top, right, bottom]
static void getMargin(ComponentEntry& entry, float margin[4]) {
    margin[0] = getPropertyFloat(entry, PropertyId::MarginLeft, 0.0f);    // 左外边距
    margin[1] = getPropertyFloat(entry, PropertyId::MarginTop, 0.0f);     // 上外边距
    margin[2] = getPropertyFloat(entry, PropertyId::MarginRight, 0.0f);   // 右外边距
    margin[3] = getPropertyFloat(entry, PropertyId::MarginBottom, 0.0f); // 下外边距
}

// 获取组件的内边距
// 参数:
//   entry - 组件条目引用
//   padding - 输出参数，内边距数组[left, top, right, bottom]
static void getPadding(ComponentEntry& entry, float padding[4]) {
    padding[0] = getPropertyFloat(entry, PropertyId::PaddingLeft, 0.0f);   // 左内边距
    padding[1] = getPropertyFloat(entry, PropertyId::PaddingTop, 0.0f);   // 上内边距
    padding[2] = getPropertyFloat(entry, PropertyId::PaddingRight, 0.0f); // 右内边距
    padding[3] = getPropertyFloat(entry, PropertyId::PaddingBottom, 0.0f); // 下内边距
}

// 获取Flexbox主轴方向
// 参数: entry - 组件条目引用
// 返回值: FlexDirection枚举值
static FlexDirection getFlexDirection(ComponentEntry& entry) {
    int dir = getPropertyInt(entry, PropertyId::FlexDirection, 0);
    return static_cast<FlexDirection>(dir);
}

// 递归计算组件的布局（完整Flexbox实现）
// 参数: nodeIdx - 组件索引
void LayoutEngine::computeLayout(int32_t nodeIdx) {
    // 检查索引有效性
    if (nodeIdx < 0) return;
    if (nodeIdx >= static_cast<int32_t>(dirtyFlags_.size())) return;
    if (nodeIdx >= static_cast<int32_t>(storage_.entries_.size())) return;
    
    // 直接获取组件（因为LayoutEngine是友元）
    auto& node = storage_.entries_[nodeIdx];
    // 检查组件是否有效且可见
    if (node.id == INVALID_COMPONENT_ID || !node.visible) return;
    
    // 调试：确认函数被调用
    Logger::getInstance().debug("=== computeLayout called for node " + std::to_string(nodeIdx) + " ===");
    
    // 确保当前组件的布局尺寸非负
    if (node.layoutResult.width < 0.0f) node.layoutResult.width = 0.0f;
    if (node.layoutResult.height < 0.0f) node.layoutResult.height = 0.0f;
    
    // 从属性中读取当前组件的大小，只有在布局结果没有设置时才使用属性值（注意：x/y 位置是由父组件布局时设置的，不要在这里覆盖！）
    if (node.layoutResult.width <= 0.0f) {
        node.layoutResult.width = getPropertyFloat(node, PropertyId::Width, 0.0f);
    }
    if (node.layoutResult.height <= 0.0f) {
        node.layoutResult.height = getPropertyFloat(node, PropertyId::Height, 0.0f);
    }
    
    // 获取容器尺寸（不强制设置默认值，保留组件自己设置的尺寸）
    float containerWidth = node.layoutResult.width;
    float containerHeight = node.layoutResult.height;
    
    // 只有在没有设置任何尺寸时才使用默认值（为了向后兼容）
    if (containerWidth <= 0 && containerHeight <= 0) {
        containerWidth = 800.0f;
        containerHeight = 600.0f;
        node.layoutResult.width = containerWidth;
        node.layoutResult.height = containerHeight;
    }
    
    // 再次确保非负
    if (node.layoutResult.width < 0.0f) node.layoutResult.width = 0.0f;
    if (node.layoutResult.height < 0.0f) node.layoutResult.height = 0.0f;
    
    // 获取内边距
    float padding[4];
    getPadding(node, padding);
    
    // 可用空间（减去内边距）
    float availableWidth = containerWidth - padding[0] - padding[2];
    float availableHeight = containerHeight - padding[1] - padding[3];
    
    // 特殊处理：零空间布局
    if (availableWidth <= 0) availableWidth = 0.0f;
    if (availableHeight <= 0) availableHeight = 0.0f;
    
    // 获取Flexbox主轴方向
    FlexDirection flexDir = getFlexDirection(node);
    
    // 收集所有可见子组件的索引
    std::vector<int32_t> visibleChildren;
    for (int32_t childIdx : node.childrenIndices) {
        if (childIdx < 0 || childIdx >= static_cast<int32_t>(storage_.entries_.size())) continue;
        auto& child = storage_.entries_[childIdx];
        if (child.id == INVALID_COMPONENT_ID || !child.visible) continue;
        visibleChildren.push_back(childIdx);
    }
    
    // 输出调试信息
    Logger::getInstance().debug("computeLayout: node=" + std::to_string(nodeIdx) + 
                                " childrenCount=" + std::to_string(node.childrenIndices.size()) +
                                " visibleCount=" + std::to_string(visibleChildren.size()));
    
    // 如果没有子组件，直接返回
    if (visibleChildren.empty()) {
        dirtyFlags_[nodeIdx].selfDirty = false;
        dirtyFlags_[nodeIdx].childrenDirty = false;
        return;
    }
    
    // 第一遍：收集所有子组件的flex属性和基准尺寸
    // 用于后续计算flexGrow和flexShrink分配
    struct FlexItem {
        int32_t index;          // 子组件索引
        float flexGrow;        // Flex放大因子
        float flexShrink;      // Flex收缩因子
        float flexBasis;       // Flex基准尺寸
        float baseSize;        // 基础尺寸（width或height属性）
        float currentSize;     // 当前尺寸
        float margin[4];       // 外边距
    };
    
    std::vector<FlexItem> items;
    items.reserve(visibleChildren.size());
    
    float totalFlexGrow = 0.0f;      // FlexGrow总和
    float totalFlexShrink = 0.0f;    // FlexShrink总和
    
    // 收集每个子组件的信息
    for (int32_t childIdx : visibleChildren) {
        auto& child = storage_.entries_[childIdx];
        
        FlexItem item;
        item.index = childIdx;
        
        // 获取Flex属性
        item.flexGrow = getPropertyFloat(child, PropertyId::FlexGrow, 0.0f);
        item.flexShrink = getPropertyFloat(child, PropertyId::FlexShrink, 1.0f);
        item.flexBasis = getPropertyFloat(child, PropertyId::FlexBasis, 0.0f);
        
        // 收集flexGrow和flexShrink总和
        totalFlexGrow += item.flexGrow;
        totalFlexShrink += item.flexShrink;
        
        // 获取基础尺寸（根据flex方向决定使用width还是height）
        if (flexDir == FlexDirection::Row) {
            // 水平方向：使用width作为主轴尺寸
            if (item.flexBasis > 0) {
                item.baseSize = item.flexBasis;
            } else {
                item.baseSize = getPropertyFloat(child, PropertyId::Width, 100.0f);
            }
        } else {
            // 垂直方向：使用height作为主轴尺寸
            if (item.flexBasis > 0) {
                item.baseSize = item.flexBasis;
            } else {
                item.baseSize = getPropertyFloat(child, PropertyId::Height, 30.0f);
            }
        }
        
        // 获取外边距
        getMargin(child, item.margin);
        
        // 基础尺寸不能为负
        item.baseSize = std::max(0.0f, item.baseSize);
        item.currentSize = item.baseSize;
        
        items.push_back(item);
    }
    
    // 确定主轴方向
    bool isMainAxisHorizontal = (flexDir == FlexDirection::Row);
    float mainAxisSize = isMainAxisHorizontal ? availableWidth : availableHeight;
    
    // 计算总基础尺寸和总外边距（根据主轴方向选择正确的外边距）
    float totalBaseSize = 0.0f;
    for (auto& item : items) {
        if (isMainAxisHorizontal) {
            totalBaseSize += item.baseSize + item.margin[0] + item.margin[2];
        } else {
            totalBaseSize += item.baseSize + item.margin[1] + item.margin[3];
        }
    }
    
    // 处理极端情况：主轴尺寸为负数（不应该发生，但防御性检查）
    // 这种情况下不执行零空间布局，而是继续正常计算
    if (mainAxisSize < 0.0f) {
        mainAxisSize = 0.0f;
    }
    
    // 第二遍：根据FlexGrow和FlexShrink分配空间
    if (isMainAxisHorizontal) {
        // 水平主轴布局
        
        // 计算初始分配后的空间
        float initialUsedSpace = totalBaseSize;
        float remainingSpace = mainAxisSize - initialUsedSpace;
        
        if (remainingSpace > 0.001f && totalFlexGrow > 0.001f) {
            // 有剩余空间且有flexGrow，按比例分配
            for (auto& item : items) {
                if (item.flexGrow > 0.001f) {
                    // flexGrow分配比例
                    float growAmount = (item.flexGrow / totalFlexGrow) * remainingSpace;
                    item.currentSize += growAmount;
                    item.currentSize = std::max(0.0f, item.currentSize);
                }
            }
        } else if (remainingSpace < -0.001f && totalFlexShrink > 0.001f) {
            // 空间不足且有flexShrink，按比例收缩
            float overflow = -remainingSpace;
            float shrinkProportionTotal = 0.0f;
            
            // 计算加权收缩总和
            for (auto& item : items) {
                if (item.flexShrink > 0.001f) {
                    shrinkProportionTotal += item.flexShrink * item.baseSize;
                }
            }
            
            if (shrinkProportionTotal > 0.001f) {
                for (auto& item : items) {
                    if (item.flexShrink > 0.001f && item.baseSize > 0.001f) {
                        // 收缩量 = (flexShrink * baseSize) / total * overflow
                        float shrinkAmount = (item.flexShrink * item.baseSize) / shrinkProportionTotal * overflow;
                        item.currentSize = std::max(0.0f, item.baseSize - shrinkAmount);
                    }
                }
            }
        }
        
        // 第三遍：确定每个子组件的位置
        float mainAxisOffset = padding[0]; // 从左内边距开始
        float crossAxisOffset = padding[1]; // 从上内边距开始
        
        Logger::getInstance().debug("  [水平布局] padding=(" + std::to_string(padding[0]) + "," + std::to_string(padding[1]) + "," + std::to_string(padding[2]) + "," + std::to_string(padding[3]) + "), availableWidth=" + std::to_string(mainAxisSize));
        
        for (size_t i = 0; i < items.size(); ++i) {
            auto& item = items[i];
            auto& child = storage_.entries_[item.index];
            
            // 设置位置
            child.layoutResult.x = mainAxisOffset + item.margin[0];
            child.layoutResult.y = crossAxisOffset + item.margin[1];
            
            // 设置尺寸
            // 主轴尺寸（由flex计算决定）
            child.layoutResult.width = item.currentSize;
            // 交叉轴尺寸使用Height属性，或者直接使用设置的值
            float childHeight = getPropertyFloat(child, PropertyId::Height, 30.0f);
            if (childHeight <= 0) childHeight = 30.0f;
            child.layoutResult.height = childHeight;
            
            Logger::getInstance().debug("  [水平布局] 子组件" + std::to_string(item.index) + 
                ": rect=(" + std::to_string(static_cast<int>(child.layoutResult.x)) + "," + 
                std::to_string(static_cast<int>(child.layoutResult.y)) + "," + 
                std::to_string(static_cast<int>(child.layoutResult.width)) + "x" + 
                std::to_string(static_cast<int>(child.layoutResult.height)) + 
                "), margin=(" + std::to_string(item.margin[0]) + "," + std::to_string(item.margin[1]) + "," + 
                std::to_string(item.margin[2]) + "," + std::to_string(item.margin[3]) + ")");
            
            // 移动到下一个位置
            mainAxisOffset += item.currentSize + item.margin[2];
        }
    } else {
        // 垂直主轴布局
        
        // 计算初始分配后的空间
        float initialUsedSpace = totalBaseSize;
        float remainingSpace = mainAxisSize - initialUsedSpace;
        
        if (remainingSpace > 0.001f && totalFlexGrow > 0.001f) {
            // 有剩余空间且有flexGrow，按比例分配
            for (auto& item : items) {
                if (item.flexGrow > 0.001f) {
                    // flexGrow分配比例
                    float growAmount = (item.flexGrow / totalFlexGrow) * remainingSpace;
                    item.currentSize += growAmount;
                    item.currentSize = std::max(0.0f, item.currentSize);
                }
            }
        } else if (remainingSpace < -0.001f && totalFlexShrink > 0.001f) {
            // 空间不足且有flexShrink，按比例收缩
            float overflow = -remainingSpace;
            float shrinkProportionTotal = 0.0f;
            
            // 计算加权收缩总和
            for (auto& item : items) {
                if (item.flexShrink > 0.001f) {
                    shrinkProportionTotal += item.flexShrink * item.baseSize;
                }
            }
            
            if (shrinkProportionTotal > 0.001f) {
                for (auto& item : items) {
                    if (item.flexShrink > 0.001f && item.baseSize > 0.001f) {
                        // 收缩量 = (flexShrink * baseSize) / total * overflow
                        float shrinkAmount = (item.flexShrink * item.baseSize) / shrinkProportionTotal * overflow;
                        item.currentSize = std::max(0.0f, item.baseSize - shrinkAmount);
                    }
                }
            }
        }
        
        // 第三遍：确定每个子组件的位置
        float mainAxisOffset = padding[1]; // 从上内边距开始
        float crossAxisOffset = padding[0]; // 从左内边距开始
        
        Logger::getInstance().debug("  [垂直布局] padding=(" + std::to_string(padding[0]) + "," + std::to_string(padding[1]) + "," + std::to_string(padding[2]) + "," + std::to_string(padding[3]) + "), availableHeight=" + std::to_string(mainAxisSize));
        
        for (size_t i = 0; i < items.size(); ++i) {
            auto& item = items[i];
            auto& child = storage_.entries_[item.index];
            
            // 设置位置
            child.layoutResult.x = crossAxisOffset + item.margin[0];
            child.layoutResult.y = mainAxisOffset + item.margin[1];
            
            // 设置尺寸
            // 交叉轴尺寸使用Width属性
            float childWidth = getPropertyFloat(child, PropertyId::Width, 100.0f);
            if (childWidth <= 0) childWidth = 100.0f;
            child.layoutResult.width = childWidth;
            // 主轴尺寸（由flex计算决定）
            child.layoutResult.height = item.currentSize;
            
            Logger::getInstance().debug("  [垂直布局] 子组件" + std::to_string(item.index) + 
                ": rect=(" + std::to_string(static_cast<int>(child.layoutResult.x)) + "," + 
                std::to_string(static_cast<int>(child.layoutResult.y)) + "," + 
                std::to_string(static_cast<int>(child.layoutResult.width)) + "x" + 
                std::to_string(static_cast<int>(child.layoutResult.height)) + 
                "), margin=(" + std::to_string(item.margin[0]) + "," + std::to_string(item.margin[1]) + "," + 
                std::to_string(item.margin[2]) + "," + std::to_string(item.margin[3]) + ")");
            
            // 移动到下一个位置
            mainAxisOffset += item.currentSize + item.margin[3];
        }
    }
    
    // 输出布局日志
    ComponentHandle handle{nodeIdx, node.generation};
    Logger::getInstance().logLayout(handle, node.layoutResult.x, node.layoutResult.y, 
                                    node.layoutResult.width, node.layoutResult.height);
    
    // 递归计算所有子组件的布局
    printf("\n[LAYOUT DEBUG] Recursing for %d children of node %d\n", (int)visibleChildren.size(), (int)nodeIdx);
    Logger::getInstance().info(">>> Recursively computing layout for " + std::to_string(visibleChildren.size()) + " children of node " + std::to_string(nodeIdx));
    for (int32_t childIdx : visibleChildren) {
        auto& child = storage_.entries_[childIdx];
        printf("[LAYOUT DEBUG] Processing child %d (id=%d, visible=%d, type=%d)\n", 
               (int)childIdx, (int)child.id, (int)child.visible, (int)child.type);
        Logger::getInstance().info(">>> Calling computeLayout for child " + std::to_string(childIdx) + 
                                   " (id=" + std::to_string(child.id) + 
                                   ", visible=" + std::to_string(child.visible) + 
                                   ", type=" + std::to_string(static_cast<int>(child.type)) + ")");
        computeLayout(childIdx);
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
