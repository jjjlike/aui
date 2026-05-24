#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include "LayoutEngine.h"
#include "EventDispatcher.h"
#include <functional>
#include <vector>

namespace aether {

/**
 * 属性变更结构
 * 
 * 用于批量处理属性变更
 */
struct PropertyChange {
    ComponentHandle handle;  // 组件句柄
    PropertyId id;              // 属性ID
    PropertyValue value;       // 新值
};

/**
 * 状态观察者接口
 * 
 * 用于监听组件状态变化
 */
class StateObserver {
public:
    virtual ~StateObserver() = default;
    
    /**
     * 属性变更回调
     * @param h 组件句柄
     * @param id 属性ID
     * @param value 新值
     */
    virtual void onPropertyChanged(ComponentHandle h, PropertyId id, const PropertyValue& value) = 0;
    
    /**
     * 组件创建回调
     * @param h 新创建的组件句柄
     */
    virtual void onComponentCreated(ComponentHandle h) = 0;
    
    /**
     * 组件销毁回调
     * @param h 被销毁的组件句柄
     */
    virtual void onComponentDestroyed(ComponentHandle h) = 0;
    
    /**
     * 布局完成回调
     */
    virtual void onLayoutComplete() = 0;
};

/**
 * 状态管理器类
 * 
 * 负责管理组件状态的变化
 * 支持批量更新、状态观察、脏标记等功能
 * 是UI框架的核心协调组件
 */
class StateManager {
public:
    /**
     * 构造函数
     * @param storage 组件存储引用
     */
    explicit StateManager(ComponentStorage& storage);
    
    /**
     * 创建组件
     * @param type 组件类型
     * @param parent 父组件句柄（可选）
     * @return 新组件句柄
     */
    ComponentHandle createComponent(ComponentType type, ComponentHandle parent = {});
    
    /**
     * 销毁组件
     * @param handle 要销毁的组件句柄
     */
    void destroyComponent(ComponentHandle handle);
    
    /**
     * 设置组件属性
     * @param h 组件句柄
     * @param id 属性ID
     * @param value 属性值
     */
    void setProperty(ComponentHandle h, PropertyId id, PropertyValue value);
    
    /**
     * 获取组件属性
     * @param h 组件句柄
     * @param id 属性ID
     * @return 属性值指针，如果不存在返回nullptr
     */
    const PropertyValue* getProperty(ComponentHandle h, PropertyId id) const;
    
    /**
     * 开始批量更新
     * 在批量操作期间，变更会被缓存
     */
    void beginBatch();
    
    /**
     * 结束批量更新
     * 应用所有缓存的变更
     */
    void endBatch();
    
    /**
     * 检查是否在批量更新中
     * @return 如果在批量更新中返回true
     */
    bool isInBatch() const { return inBatch_; }
    
    /**
     * 设置布局引擎
     * @param engine 布局引擎指针
     */
    void setLayoutEngine(LayoutEngine* engine) { layoutEngine_ = engine; }
    
    /**
     * 设置事件分发器
     * @param dispatcher 事件分发器指针
     */
    void setEventDispatcher(EventDispatcher* dispatcher) { eventDispatcher_ = dispatcher; }
    
    /**
     * 添加状态观察者
     * @param observer 观察者指针
     */
    void addObserver(StateObserver* observer);
    
    /**
     * 移除状态观察者
     * @param observer 观察者指针
     */
    void removeObserver(StateObserver* observer);
    
    /**
     * 通知属性变更
     * @param h 组件句柄
     * @param id 属性ID
     * @param value 属性值
     */
    void notifyPropertyChanged(ComponentHandle h, PropertyId id, const PropertyValue& value);
    
    /**
     * 通知组件创建
     * @param h 组件句柄
     */
    void notifyComponentCreated(ComponentHandle h);
    
    /**
     * 通知组件销毁
     * @param h 组件句柄
     */
    void notifyComponentDestroyed(ComponentHandle h);
    
    /**
     * 通知布局完成
     */
    void notifyLayoutComplete();
    
    /**
     * 获取组件存储（可变版本
     * @return 组件存储引用
     */
    ComponentStorage& getStorage() { return storage_; }
    
    /**
     * 获取组件存储（const版本
     * @return 组件存储const引用
     */
    const ComponentStorage& getStorage() const { return storage_; }
    
private:
    /**
     * 应用属性变更
     * @param change 属性变更
     */
    void applyChange(const PropertyChange& change);
    
    ComponentStorage& storage_;              // 组件存储引用
    LayoutEngine* layoutEngine_ = nullptr;   // 布局引擎指针
    EventDispatcher* eventDispatcher_ = nullptr;  // 事件分发器指针
    
    std::vector<PropertyChange> batchBuffer_;  // 批量变更缓冲区
    bool inBatch_ = false;               // 是否在批量更新中
    
    std::vector<StateObserver*> observers_;  // 观察者列表
};

}
