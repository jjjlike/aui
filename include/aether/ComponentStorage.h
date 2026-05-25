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
#include "property_id.h"
#include <bitset>
#include <optional>
#include <string>

namespace jaether {

/**
 * 属性值容器类
 * 
 * 使用std::variant存储多种类型的属性值
 * 支持的类型：整数、浮点数、布尔值、字符串、颜色、矩形、布局相关枚举
 */
struct JPropertyValue {
    using ValueType = std::variant<
        std::monostate,
        int,
        float,
        bool,
        std::string,
        JColor,
        JRect,
        JFlexDirection,
        JFlexWrap,
        JJustifyContent,
        JAlignItems,
        JAlignContent
    >;
    
    ValueType value;
    
    JPropertyValue() : value(std::monostate{}) {}
    
    template<typename T>
    JPropertyValue(T val) : value(val) {}
    
    /**
     * 检查属性是否有值
     * @return true如果不是monostate
     */
    bool hasValue() const {
        return !std::holds_alternative<std::monostate>(value);
    }
    
    /**
     * 获取属性值
     * @tparam T 属性值的类型
     * @return 属性值
     */
    template<typename T>
    T get() const {
        return std::get<T>(value);
    }
    
    /**
     * 检查属性值是否是特定类型
     * @tparam T 要检查的类型
     * @return true如果是该类型
     */
    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(value);
    }
    
    /**
     * 将属性值转换为字符串
     * @return 属性值的字符串表示
     */
    std::string toString() const;
    
    /**
     * 从字符串解析属性值
     * @param id 属性ID
     * @param str 属性值字符串
     * @return 解析后的PropertyValue
     */
    static JPropertyValue fromString(JPropertyId id, const std::string& str);
};

/**
 * 属性块类
 * 
 * 使用bitset标记属性存在性，使用vector存储属性值
 * 提供属性的读写操作
 */
struct JPropertyBlock {
    std::vector<JPropertyValue> values;
    std::bitset<MAX_PROPERTY_ID> presence;
    
    /**
     * 确保values的大小足够存储指定ID的属性
     * @param size 需要的大小
     */
    void ensureSize(size_t size) {
        if (values.size() < size) {
            values.resize(size);
        }
    }
    
    /**
     * 检查是否有指定属性
     * @param id 属性ID
     * @return true如果属性存在
     */
    bool hasProperty(JPropertyId id) const {
        return presence.test(static_cast<size_t>(id));
    }
    
    /**
     * 获取属性值
     * @param id 属性ID
     * @return 属性值指针，不存在则返回nullptr
     */
    const JPropertyValue* getProperty(JPropertyId id) const {
        size_t idx = static_cast<size_t>(id);
        if (idx < values.size() && presence.test(idx)) {
            return &values[idx];
        }
        return nullptr;
    }
    
    /**
     * 设置属性值
     * @param id 属性ID
     * @param value 新的属性值
     */
    void setProperty(JPropertyId id, JPropertyValue value) {
        size_t idx = static_cast<size_t>(id);
        ensureSize(idx + 1);
        values[idx] = std::move(value);
        presence.set(idx);
    }
    
    /**
     * 清除属性
     * @param id 属性ID
     */
    void clearProperty(JPropertyId id) {
        size_t idx = static_cast<size_t>(id);
        if (idx < values.size()) {
            values[idx] = JPropertyValue{};
            presence.reset(idx);
        }
    }
};

/**
 * 组件条目结构体 (SoA布局)
 * 
 * 存储单个组件的所有数据
 * 包括：ID、类型、布局结果、父子关系、属性、可见性等
 */
struct JComponentEntry {
    JComponentId id = INVALID_COMPONENT_ID;
    JComponentType type = JComponentType::Container;
    JRect layoutResult;
    int32_t parentIndex = -1;
    std::vector<int32_t> childrenIndices;
    JPropertyBlock properties;
    JGeneration generation = 0;
    bool visible = true;
    bool enabled = true;
    std::string debugName;
};

/**
 * 组件存储类
 * 
 * 负责组件的创建、销毁、存储和访问
 * 使用SoA布局提升缓存效率
 * 提供组件句柄和世代号机制防止悬空指针
 */
class JComponentStorage {
public:
    /**
     * 创建组件
     * @param type 组件类型
     * @param parent 父组件句柄，可选
     * @return 新创建的组件句柄
     */
    JComponentHandle createComponent(JComponentType type, JComponentHandle parent = {});
    
    /**
     * 销毁组件
     * @param handle 要销毁的组件句柄
     */
    void destroyComponent(JComponentHandle handle);
    
    /**
     * 检查句柄是否有效
     * @param handle 组件句柄
     * @return true如果句柄有效
     */
    bool isValid(JComponentHandle handle) const;
    
    /**
     * 获取组件指针
     * @param handle 组件句柄
     * @return 组件条目指针，无效则返回nullptr
     */
    JComponentEntry* getComponent(JComponentHandle handle);
    const JComponentEntry* getComponent(JComponentHandle handle) const;
    
    /**
     * 通过索引获取组件指针（仅用于内部遍历，用于快速访问）
     * @param index 槽位索引
     * @return 组件条目指针，无效则返回nullptr
     */
    JComponentEntry* getComponentByIndex(int32_t index);
    const JComponentEntry* getComponentByIndex(int32_t index) const;
    
    /**
     * 通过组件ID查找组件
     * @param id 组件ID
     * @return 组件句柄，未找到则返回无效句柄
     */
    JComponentHandle findById(JComponentId id) const;
    
    /**
     * 获取根组件
     * @return 根组件句柄
     */
    JComponentHandle getRoot() const { return root_; }
    
    /**
     * 获取组件总数（包括已销毁的）
     * @return 组件总数量
     */
    size_t size() const { return entries_.size(); }
    
    /**
     * 获取活动组件数量
     * @return 当前活动的组件数量
     */
    size_t activeCount() const;
    
    /**
     * 遍历所有活动组件（非const版本）
     * @tparam Func 回调函数类型
     * @param func 回调函数，参数为组件句柄
     */
    template<typename Func>
    void forEach(Func&& func) {
        for (size_t i = 0; i < entries_.size(); ++i) {
            if (entries_[i].id != INVALID_COMPONENT_ID) {
                func(JComponentHandle{static_cast<int32_t>(i), entries_[i].generation});
            }
        }
    }
    
    /**
     * 遍历所有活动组件（const版本）
     * @tparam Func 回调函数类型
     * @param func 回调函数，参数为组件句柄
     */
    template<typename Func>
    void forEach(Func&& func) const {
        for (size_t i = 0; i < entries_.size(); ++i) {
            if (entries_[i].id != INVALID_COMPONENT_ID) {
                func(JComponentHandle{static_cast<int32_t>(i), entries_[i].generation});
            }
        }
    }
    
    /**
     * 清空所有组件
     */
    void clear();
    
    /**
     * 计算组件的绝对位置
     * @param index 组件索引
     * @return 组件的绝对矩形
     */
    JRect getAbsoluteBounds(int32_t index) const;
    
    /**
     * 计算组件的绝对位置
     * @param handle 组件句柄
     * @return 组件的绝对矩形
     */
    JRect getAbsoluteBounds(JComponentHandle handle) const;
    
    /**
     * 获取组件的相对位置（相对于父组件）
     * @param handle 组件句柄
     * @return 组件的相对矩形
     */
    JRect getRelativeBounds(JComponentHandle handle) const;
    
    /**
     * 检查点是否在组件的绝对位置范围内
     * @param handle 组件句柄
     * @param x 点的x坐标
     * @param y 点的y坐标
     * @return 是否在范围内
     */
    bool containsPoint(JComponentHandle handle, float x, float y) const;
    
    /**
     * 检查点是否在组件的绝对位置范围内
     * @param handle 组件句柄
     * @param point 点
     * @return 是否在范围内
     */
    bool containsPoint(JComponentHandle handle, JPoint point) const;
    
private:
    /**
     * 分配一个槽位
     * @return 槽位索引
     */
    int32_t allocateSlot();
    
    /**
     * 释放一个槽位
     * @param index 槽位索引
     */
    void freeSlot(int32_t index);
    
    std::vector<JComponentEntry> entries_;
    std::vector<int32_t> freeList_;
    JComponentId nextId_ = 1;
    JComponentHandle root_;
    
    friend class JLayoutEngine;
};

}
