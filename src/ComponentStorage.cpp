// ComponentStorage.cpp
// 组件存储模块 - 管理所有UI组件的创建、销毁和层次结构
//
// 功能:
// - 组件的创建和销毁
// - 组件的父子关系管理
// - 组件ID分配和回收
// - 支持通过ID查找组件
// - 使用空闲链表优化内存使用

#include "aether/ComponentStorage.h"
#include "aether/Logger.h"
#include <algorithm>

namespace aether {

// 分配一个可用的组件槽位
// 返回值: 分配到的槽位索引
int32_t ComponentStorage::allocateSlot() {
    // 优先从空闲链表中回收槽位
    if (!freeList_.empty()) {
        int32_t slot = freeList_.back();
        freeList_.pop_back();
        return slot;
    }
    // 如果没有空闲槽位，创建新的槽位
    int32_t newIndex = static_cast<int32_t>(entries_.size());
    entries_.emplace_back();
    return newIndex;
}

// 释放指定索引的组件槽位
// 参数: index - 要释放的槽位索引
void ComponentStorage::freeSlot(int32_t index) {
    // 检查索引是否有效
    if (index >= 0 && index < static_cast<int32_t>(entries_.size())) {
        // 将组件ID标记为无效
        entries_[index].id = INVALID_COMPONENT_ID;
        // 增加世代号，使旧的句柄失效
        entries_[index].generation++;
        // 将索引添加到空闲链表
        freeList_.push_back(index);
    }
}

// 创建一个新的组件
// 参数: 
//   type - 组件类型
//   parent - 父组件句柄（可选）
// 返回值: 新创建的组件句柄
ComponentHandle ComponentStorage::createComponent(ComponentType type, ComponentHandle parent) {
    // 分配槽位
    int32_t slot = allocateSlot();
    auto& entry = entries_[slot];
    
    // 初始化组件
    entry.id = nextId_++;
    entry.type = type;
    entry.generation = 1;
    entry.visible = true;
    entry.enabled = true;
    entry.parentIndex = -1;
    entry.childrenIndices.clear();
    
    // 设置父子关系
    if (parent.isValid()) {
        entry.parentIndex = parent.index;
        entries_[parent.index].childrenIndices.push_back(slot);
    } else if (!root_.isValid()) {
        // 如果没有父组件且根组件未设置，设为根组件
        root_ = ComponentHandle{slot, entry.generation};
    }
    
    ComponentHandle handle{slot, entry.generation};
    
    // 输出日志
    Logger::getInstance().logComponentCreate(handle, type, parent);
    
    return handle;
}

// 销毁指定的组件及其所有子组件
// 参数: handle - 要销毁的组件句柄
void ComponentStorage::destroyComponent(ComponentHandle handle) {
    // 检查句柄是否有效
    if (!isValid(handle)) return;
    
    auto& entry = entries_[handle.index];
    
    // 递归销毁所有子组件
    for (int32_t childIdx : entry.childrenIndices) {
        destroyComponent(ComponentHandle{childIdx, entries_[childIdx].generation});
    }
    
    // 从父组件的子列表中移除
    if (entry.parentIndex >= 0) {
        auto& parent = entries_[entry.parentIndex];
        auto it = std::find(parent.childrenIndices.begin(), parent.childrenIndices.end(), handle.index);
        if (it != parent.childrenIndices.end()) {
            parent.childrenIndices.erase(it);
        }
    }
    
    // 如果是根组件，清空根引用
    if (root_.index == handle.index) {
        root_ = ComponentHandle{};
    }
    
    // 输出日志
    Logger::getInstance().logComponentDestroy(handle);
    
    // 释放槽位
    freeSlot(handle.index);
}

// 检查组件句柄是否有效
// 参数: handle - 要检查的组件句柄
// 返回值: 有效返回true，否则返回false
bool ComponentStorage::isValid(ComponentHandle handle) const {
    // 检查索引范围
    if (handle.index < 0 || handle.index >= static_cast<int32_t>(entries_.size())) {
        return false;
    }
    // 检查ID和世代号
    return entries_[handle.index].id != INVALID_COMPONENT_ID &&
           entries_[handle.index].generation == handle.generation;
}

// 获取组件的可变指针
// 参数: handle - 组件句柄
// 返回值: 组件指针，无效句柄返回nullptr
ComponentEntry* ComponentStorage::getComponent(ComponentHandle handle) {
    if (!isValid(handle)) return nullptr;
    return &entries_[handle.index];
}

// 获取组件的常量指针
// 参数: handle - 组件句柄
// 返回值: 组件指针，无效句柄返回nullptr
const ComponentEntry* ComponentStorage::getComponent(ComponentHandle handle) const {
    if (!isValid(handle)) return nullptr;
    return &entries_[handle.index];
}

// 通过索引获取组件的可变指针
// 参数: index - 槽位索引
// 返回值: 组件指针，无效索引返回nullptr
ComponentEntry* ComponentStorage::getComponentByIndex(int32_t index) {
    if (index < 0 || index >= static_cast<int32_t>(entries_.size())) return nullptr;
    if (entries_[index].id == INVALID_COMPONENT_ID) return nullptr;
    return &entries_[index];
}

// 通过索引获取组件的常量指针
// 参数: index - 槽位索引
// 返回值: 组件指针，无效索引返回nullptr
const ComponentEntry* ComponentStorage::getComponentByIndex(int32_t index) const {
    if (index < 0 || index >= static_cast<int32_t>(entries_.size())) return nullptr;
    if (entries_[index].id == INVALID_COMPONENT_ID) return nullptr;
    return &entries_[index];
}

// 获取活动组件数量（活跃的）
size_t ComponentStorage::activeCount() const {
    size_t count = 0;
    for (const auto& entry : entries_) {
        if (entry.id != INVALID_COMPONENT_ID) {
            count++;
        }
    }
    return count;
}

// 通过组件ID查找组件
// 参数: id - 组件ID
// 返回值: 找到的组件句柄，未找到返回无效句柄
ComponentHandle ComponentStorage::findById(ComponentId id) const {
    // 遍历所有组件查找
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].id == id) {
            return ComponentHandle{static_cast<int32_t>(i), entries_[i].generation};
        }
    }
    return ComponentHandle{};
}

// 清空所有组件
void ComponentStorage::clear() {
    entries_.clear();
    freeList_.clear();
    nextId_ = 1;
    root_ = ComponentHandle{};
}

} // namespace aether
