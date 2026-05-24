#pragma once

#include "types.h"
#include "property_id.h"
#include <bitset>
#include <optional>
#include <string>

namespace aether {

/**
 * 属性值容器类
 * 
 * 使用std::variant存储多种类型的属性值
 * 支持的类型：整数、浮点数、布尔值、字符串、颜色、矩形、布局相关枚举
 */
struct PropertyValue {
    using ValueType = std::variant<
        std::monostate,
        int,
        float,
        bool,
        std::string,
        Color,
        Rect,
        FlexDirection,
        FlexWrap,
        JustifyContent,
        AlignItems,
        AlignContent
    >;
    
    ValueType value;
    
    PropertyValue() : value(std::monostate{}) {}
    
    template<typename T>
    PropertyValue(T val) : value(val) {}
    
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
    static PropertyValue fromString(PropertyId id, const std::string& str);
};

/**
 * 属性块类
 * 
 * 使用bitset标记属性存在性，使用vector存储属性值
 * 提供属性的读写操作
 */
struct PropertyBlock {
    std::vector<PropertyValue> values;
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
    bool hasProperty(PropertyId id) const {
        return presence.test(static_cast<size_t>(id));
    }
    
    /**
     * 获取属性值
     * @param id 属性ID
     * @return 属性值指针，不存在则返回nullptr
     */
    const PropertyValue* getProperty(PropertyId id) const {
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
    void setProperty(PropertyId id, PropertyValue value) {
        size_t idx = static_cast<size_t>(id);
        ensureSize(idx + 1);
        values[idx] = std::move(value);
        presence.set(idx);
    }
    
    /**
     * 清除属性
     * @param id 属性ID
     */
    void clearProperty(PropertyId id) {
        size_t idx = static_cast<size_t>(id);
        if (idx < values.size()) {
            values[idx] = PropertyValue{};
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
struct ComponentEntry {
    ComponentId id = INVALID_COMPONENT_ID;
    ComponentType type = ComponentType::Container;
    Rect layoutResult;
    int32_t parentIndex = -1;
    std::vector<int32_t> childrenIndices;
    PropertyBlock properties;
    Generation generation = 0;
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
class ComponentStorage {
public:
    /**
     * 创建组件
     * @param type 组件类型
     * @param parent 父组件句柄，可选
     * @return 新创建的组件句柄
     */
    ComponentHandle createComponent(ComponentType type, ComponentHandle parent = {});
    
    /**
     * 销毁组件
     * @param handle 要销毁的组件句柄
     */
    void destroyComponent(ComponentHandle handle);
    
    /**
     * 检查句柄是否有效
     * @param handle 组件句柄
     * @return true如果句柄有效
     */
    bool isValid(ComponentHandle handle) const;
    
    /**
     * 获取组件指针
     * @param handle 组件句柄
     * @return 组件条目指针，无效则返回nullptr
     */
    ComponentEntry* getComponent(ComponentHandle handle);
    const ComponentEntry* getComponent(ComponentHandle handle) const;
    
    /**
     * 通过索引获取组件指针（仅用于内部遍历，用于快速访问）
     * @param index 槽位索引
     * @return 组件条目指针，无效则返回nullptr
     */
    ComponentEntry* getComponentByIndex(int32_t index);
    const ComponentEntry* getComponentByIndex(int32_t index) const;
    
    /**
     * 通过组件ID查找组件
     * @param id 组件ID
     * @return 组件句柄，未找到则返回无效句柄
     */
    ComponentHandle findById(ComponentId id) const;
    
    /**
     * 获取根组件
     * @return 根组件句柄
     */
    ComponentHandle getRoot() const { return root_; }
    
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
                func(ComponentHandle{static_cast<int32_t>(i), entries_[i].generation});
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
                func(ComponentHandle{static_cast<int32_t>(i), entries_[i].generation});
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
    Rect getAbsoluteBounds(int32_t index) const;
    
    /**
     * 计算组件的绝对位置
     * @param handle 组件句柄
     * @return 组件的绝对矩形
     */
    Rect getAbsoluteBounds(ComponentHandle handle) const;
    
    /**
     * 获取组件的相对位置（相对于父组件）
     * @param handle 组件句柄
     * @return 组件的相对矩形
     */
    Rect getRelativeBounds(ComponentHandle handle) const;
    
    /**
     * 检查点是否在组件的绝对位置范围内
     * @param handle 组件句柄
     * @param x 点的x坐标
     * @param y 点的y坐标
     * @return 是否在范围内
     */
    bool containsPoint(ComponentHandle handle, float x, float y) const;
    
    /**
     * 检查点是否在组件的绝对位置范围内
     * @param handle 组件句柄
     * @param point 点
     * @return 是否在范围内
     */
    bool containsPoint(ComponentHandle handle, Point point) const;
    
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
    
    std::vector<ComponentEntry> entries_;
    std::vector<int32_t> freeList_;
    ComponentId nextId_ = 1;
    ComponentHandle root_;
    
    friend class LayoutEngine;
};

}
