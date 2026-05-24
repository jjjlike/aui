#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include "property_id.h"
#include <vector>

namespace aether {

/**
 * 脏标记结构体
 * 
 * 用于标记组件是否需要重新布局
 * selfDirty - 自身布局脏标记
 * childrenDirty - 子组件布局脏标记
 */
struct DirtyFlags {
    bool selfDirty = false;       // 自身布局是否需要重新计算
    bool childrenDirty = false;   // 子组件布局是否需要重新计算
};

/**
 * 布局引擎类
 * 
 * 负责处理组件的布局计算
 * 使用Flexbox布局算法
 * 支持脏标记机制，只重新布局需要更新的组件
 */
class LayoutEngine {
public:
    /**
     * 构造函数
     * @param storage 组件存储引用
     */
    explicit LayoutEngine(ComponentStorage& storage);
    
    /**
     * 标记组件为脏状态
     * @param h 组件句柄
     * @param changedProp 改变的属性ID
     */
    void markDirty(ComponentHandle h, PropertyId changedProp);
    
    /**
     * 标记组件为脏状态（重载版本）
     * @param h 组件句柄
     */
    void markDirty(ComponentHandle h);
    
    /**
     * 检查组件是否为脏状态
     * @param h 组件句柄
     * @return 如果组件是脏状态返回true
     */
    bool isDirty(ComponentHandle h) const;
    
    /**
     * 执行必要的重新布局
     * 只处理被标记为脏的组件
     */
    void relayoutIfNeeded();
    
    /**
     * 强制重新布局所有组件
     */
    void forceRelayout();
    
    /**
     * 设置布局引擎模式（兼容旧代码接口）
     * @param mode 布局模式
     */
    void setMode(LayoutEngineMode mode) { /* 兼容旧代码 */ }
    
    /**
     * 获取布局引擎模式
     * @return 当前布局模式（总是返回Normal）
     */
    LayoutEngineMode getMode() const { return LayoutEngineMode::Normal; }
    
    /**
     * 获取总布局时间
     * @return 累计布局时间（毫秒）
     */
    float getTotalLayoutTime() const { return totalLayoutTime_; }
    
    /**
     * 重置布局时间统计
     */
    void resetMetrics();
    
private:
    /**
     * 计算指定节点的布局
     * @param nodeIdx 节点索引
     */
    void computeLayout(int32_t nodeIdx);
    
    /**
     * 清除所有脏标记
     */
    void clearDirtyFlags();
    
    ComponentStorage& storage_;              // 组件存储引用
    std::vector<DirtyFlags> dirtyFlags_;    // 脏标记数组
    float totalLayoutTime_ = 0;             // 累计布局时间
};

}
