// StateManager.cpp
// 状态管理器模块 - 负责管理UI状态的变更和通知
//
// 功能:
// - 组件创建和销毁
// - 属性设置和获取
// - 批量操作支持
// - 观察者模式 - 通知状态变更
// - 与布局引擎和事件分发器集成

#include "aether/StateManager.h"

namespace aether {

// 状态管理器构造函数
// 参数: storage - 组件存储引用
StateManager::StateManager(ComponentStorage& storage)
    : storage_(storage) {
}

// 创建组件
// 参数:
//   type - 组件类型
//   parent - 父组件句柄
// 返回值: 新创建的组件句柄
ComponentHandle StateManager::createComponent(ComponentType type, ComponentHandle parent) {
    ComponentHandle handle = storage_.createComponent(type, parent);
    
    // 触发布局更新
    if (layoutEngine_) {
        layoutEngine_->markDirty(handle);
        layoutEngine_->relayoutIfNeeded();
    }
    
    // 通知观察者
    notifyComponentCreated(handle);
    
    return handle;
}

// 销毁组件
// 参数: handle - 要销毁的组件句柄
void StateManager::destroyComponent(ComponentHandle handle) {
    // 先通知观察者
    notifyComponentDestroyed(handle);
    storage_.destroyComponent(handle);
    
    // 触发布局更新
    if (layoutEngine_) {
        layoutEngine_->relayoutIfNeeded();
    }
}

// 设置组件属性
// 参数:
//   h - 组件句柄
//   id - 属性ID
//   value - 属性值
void StateManager::setProperty(ComponentHandle h, PropertyId id, PropertyValue value) {
    auto* entry = storage_.getComponent(h);
    if (!entry) return;
    
    // 批量模式下缓存变更
    if (inBatch_) {
        batchBuffer_.push_back({h, id, std::move(value)});
    } else {
        // 直接应用变更
        PropertyChange change{h, id, std::move(value)};
        applyChange(change);
        
        // 触发布局更新（如果需要）
        if (layoutEngine_) {
            if (isLayoutAffectingProperty(id)) {
                layoutEngine_->markDirty(h, id);
                layoutEngine_->relayoutIfNeeded();
            }
        }
        
        // 更新事件分发器
        if (eventDispatcher_) {
            eventDispatcher_->onLayoutComplete();
        }
        
        // 通知观察者
        notifyPropertyChanged(h, id, entry->properties.getProperty(id) ? 
            *entry->properties.getProperty(id) : PropertyValue{});
    }
}

// 获取组件属性
// 参数:
//   h - 组件句柄
//   id - 属性ID
// 返回值: 属性值指针，不存在则返回nullptr
const PropertyValue* StateManager::getProperty(ComponentHandle h, PropertyId id) const {
    auto* entry = storage_.getComponent(h);
    if (!entry) return nullptr;
    return entry->properties.getProperty(id);
}

// 开始批量操作
void StateManager::beginBatch() {
    inBatch_ = true;
    batchBuffer_.clear();
}

// 结束批量操作，应用所有缓存的变更
void StateManager::endBatch() {
    inBatch_ = false;
    
    bool needsLayout = false;
    
    // 保存要通知的变更
    std::vector<PropertyChange> changesToNotify = batchBuffer_;
    
    // 应用所有变更
    for (auto& change : batchBuffer_) {
        applyChange(change);
        
        // 检查是否需要布局
        if (layoutEngine_ && isLayoutAffectingProperty(change.id)) {
            needsLayout = true;
        }
    }
    
    batchBuffer_.clear();
    
    // 触发布局更新（如果需要）
    if (needsLayout && layoutEngine_) {
        layoutEngine_->relayoutIfNeeded();
    }
    
    // 更新事件分发器
    if (eventDispatcher_) {
        eventDispatcher_->onLayoutComplete();
    }
    
    // 通知观察者
    for (auto& change : changesToNotify) {
        auto* entry = storage_.getComponent(change.handle);
        if (entry) {
            notifyPropertyChanged(change.handle, change.id, 
                entry->properties.getProperty(change.id) ? 
                *entry->properties.getProperty(change.id) : PropertyValue{});
        }
    }
}

// 应用属性变更
// 参数: change - 属性变更对象
void StateManager::applyChange(const PropertyChange& change) {
    auto* entry = storage_.getComponent(change.handle);
    if (!entry) return;
    
    // 处理特殊属性
    switch (change.id) {
        case PropertyId::Visible:
            if (change.value.is<bool>()) {
                entry->visible = change.value.get<bool>();
            }
            break;
        case PropertyId::Enabled:
            if (change.value.is<bool>()) {
                entry->enabled = change.value.get<bool>();
            }
            break;
        default:
            // 普通属性
            entry->properties.setProperty(change.id, change.value);
            break;
    }
}

// 添加观察者
// 参数: observer - 观察者指针
void StateManager::addObserver(StateObserver* observer) {
    if (observer) {
        observers_.push_back(observer);
    }
}

// 移除观察者
// 参数: observer - 观察者指针
void StateManager::removeObserver(StateObserver* observer) {
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

// 通知属性变更
// 参数:
//   h - 组件句柄
//   id - 属性ID
//   value - 属性值
void StateManager::notifyPropertyChanged(ComponentHandle h, PropertyId id, const PropertyValue& value) {
    for (auto* observer : observers_) {
        observer->onPropertyChanged(h, id, value);
    }
}

// 通知组件创建
// 参数: h - 组件句柄
void StateManager::notifyComponentCreated(ComponentHandle h) {
    for (auto* observer : observers_) {
        observer->onComponentCreated(h);
    }
}

// 通知组件销毁
// 参数: h - 组件句柄
void StateManager::notifyComponentDestroyed(ComponentHandle h) {
    for (auto* observer : observers_) {
        observer->onComponentDestroyed(h);
    }
}

// 通知布局完成
void StateManager::notifyLayoutComplete() {
    for (auto* observer : observers_) {
        observer->onLayoutComplete();
    }
}

} // namespace aether
