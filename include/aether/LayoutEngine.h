// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include "property_id.h"
#include <vector>

namespace jaether {

/**
 * 脏标记结构体
 * 
 * 用于标记组件是否需要重新布局
 * selfDirty - 自身布局脏标记
 * childrenDirty - 子组件布局脏标记
 */
struct JDirtyFlags {
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
class JLayoutEngine {
public:
    /**
     * 构造函数
     * @param storage 组件存储引用
     */
    explicit JLayoutEngine(JComponentStorage& storage);
    
    /**
     * 标记组件为脏状态
     * @param h 组件句柄
     * @param changedProp 改变的属性ID
     */
    void markDirty(JComponentHandle h, JPropertyId changedProp);
    
    /**
     * 标记组件为脏状态（重载版本）
     * @param h 组件句柄
     */
    void markDirty(JComponentHandle h);
    
    /**
     * 检查组件是否为脏状态
     * @param h 组件句柄
     * @return 如果组件是脏状态返回true
     */
    bool isDirty(JComponentHandle h) const;
    
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
     * 设置布局引擎模式
     * @param mode 布局模式
     */
    void setMode(JLayoutEngineMode mode);
    
    /**
     * 获取布局引擎模式
     * @return 当前布局模式
     */
    JLayoutEngineMode getMode() const;
    
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
    
    JComponentStorage& storage_;              // 组件存储引用
    std::vector<JDirtyFlags> dirtyFlags_;    // 脏标记数组
    float totalLayoutTime_ = 0;             // 累计布局时间
    JLayoutEngineMode mode_ = JLayoutEngineMode::Normal; // 布局引擎模式
};

}
