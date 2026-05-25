// SurfaceManager.cpp
// Surface生命周期管理模块 - 管理A2UI Surface的创建、销毁和ID映射
//
// 功能:
// - 创建和删除Surface
// - 维护A2UI字符串ID与jaether JComponentHandle之间的双向映射
// - 支持多Surface并存，每个Surface拥有独立的组件命名空间
// - 提供组件查找和遍历接口

#include "aether/SurfaceManager.h"

namespace jaether {

// 构造函数：存储组件存储引用
JJSurfaceManager::JJSurfaceManager(JComponentStorage& storage)
    : storage_(storage) {
}

// 创建新的Surface
// 如果surfaceId已存在则返回false，保证Surface ID唯一性
bool JJSurfaceManager::createSurface(const std::string& surfaceId, const std::string& catalogId) {
    // 条件1: surfaceId不能为空
    if (surfaceId.empty()) {
        return false;
    }
    
    // 条件2: surfaceId不能与已有Surface重复
    if (hasSurface(surfaceId)) {
        return false;
    }
    
    // 创建新的Surface记录
    Surface surface;
    surface.id = surfaceId;
    surface.catalogId = catalogId;
    surface.rootHandle = JComponentHandle{};  // 初始时根组件尚未设置
    
    surfaces_[surfaceId] = surface;
    return true;
}

// 删除Surface及其所有组件映射
// 注意：此方法仅清理映射关系，不销毁组件本身
// 组件的生命周期由JComponentStorage管理
void JJSurfaceManager::deleteSurface(const std::string& surfaceId) {
    auto it = surfaces_.find(surfaceId);
    if (it != surfaces_.end()) {
        surfaces_.erase(it);
    }
}

// 注册组件的A2UI ID映射
// 建立双向映射：A2UI字符串ID ↔ jaether组件句柄
void JJSurfaceManager::registerComponent(const std::string& surfaceId, 
                                          const std::string& a2uiId, 
                                          JComponentHandle handle) {
    // 前置条件检查: surface必须存在
    if (!hasSurface(surfaceId)) {
        return;
    }
    // 前置条件检查: handle必须有效
    if (!handle.isValid()) {
        return;
    }
    // 前置条件检查: a2uiId不能为空
    if (a2uiId.empty()) {
        return;
    }
    
    auto& surface = surfaces_[surfaceId];
    
    // 如果已存在相同A2UI ID，先注销旧映射
    auto oldIt = surface.a2uiToHandle.find(a2uiId);
    if (oldIt != surface.a2uiToHandle.end()) {
        // 清理旧的反向映射
        surface.handleToA2UI.erase(oldIt->second.index);
    }
    
    // 建立正向映射: A2UI ID → handle
    surface.a2uiToHandle[a2uiId] = handle;
    
    // 建立反向映射: handle索引 → A2UI ID
    surface.handleToA2UI[handle.index] = a2uiId;
}

// 注销组件映射
// 移除A2UI ID与组件句柄之间的关联关系
void JJSurfaceManager::unregisterComponent(const std::string& surfaceId, 
                                            const std::string& a2uiId) {
    if (!hasSurface(surfaceId)) {
        return;
    }
    
    auto& surface = surfaces_[surfaceId];
    auto it = surface.a2uiToHandle.find(a2uiId);
    if (it != surface.a2uiToHandle.end()) {
        // 删除反向映射
        surface.handleToA2UI.erase(it->second.index);
        // 删除正向映射
        surface.a2uiToHandle.erase(it);
    }
}

// 通过A2UI ID查找对应的jaether组件句柄
// 先在映射表中查找，然后验证句柄有效性
JComponentHandle JJSurfaceManager::findComponent(const std::string& surfaceId, 
                                                  const std::string& a2uiId) const {
    if (!hasSurface(surfaceId)) {
        return JComponentHandle{};
    }
    
    const auto& surface = surfaces_.at(surfaceId);
    auto it = surface.a2uiToHandle.find(a2uiId);
    if (it == surface.a2uiToHandle.end()) {
        return JComponentHandle{};
    }
    
    // 验证句柄是否仍然有效（组件可能已被销毁）
    if (!storage_.isValid(it->second)) {
        return JComponentHandle{};
    }
    
    return it->second;
}

// 通过jaether组件句柄查找对应的A2UI ID
// 使用句柄的index字段进行反向查询
std::string JJSurfaceManager::findA2UIId(const std::string& surfaceId, 
                                          JComponentHandle handle) const {
    if (!hasSurface(surfaceId) || !handle.isValid()) {
        return "";
    }
    
    const auto& surface = surfaces_.at(surfaceId);
    auto it = surface.handleToA2UI.find(handle.index);
    if (it != surface.handleToA2UI.end()) {
        return it->second;
    }
    return "";
}

// 获取Surface的根组件句柄
JComponentHandle JJSurfaceManager::getRootComponent(const std::string& surfaceId) const {
    if (!hasSurface(surfaceId)) {
        return JComponentHandle{};
    }
    return surfaces_.at(surfaceId).rootHandle;
}

// 设置Surface的根组件句柄
void JJSurfaceManager::setRootComponent(const std::string& surfaceId, JComponentHandle handle) {
    if (hasSurface(surfaceId)) {
        surfaces_[surfaceId].rootHandle = handle;
    }
}

// 获取Surface中所有组件的A2UI ID列表
std::vector<std::string> JJSurfaceManager::getAllComponentIds(const std::string& surfaceId) const {
    std::vector<std::string> result;
    if (!hasSurface(surfaceId)) {
        return result;
    }
    
    const auto& surface = surfaces_.at(surfaceId);
    result.reserve(surface.a2uiToHandle.size());
    for (const auto& pair : surface.a2uiToHandle) {
        result.push_back(pair.first);
    }
    return result;
}

// 获取Surface中所有组件的jaether句柄列表
std::vector<JComponentHandle> JJSurfaceManager::getAllComponentHandles(const std::string& surfaceId) const {
    std::vector<JComponentHandle> result;
    if (!hasSurface(surfaceId)) {
        return result;
    }
    
    const auto& surface = surfaces_.at(surfaceId);
    result.reserve(surface.a2uiToHandle.size());
    for (const auto& pair : surface.a2uiToHandle) {
        result.push_back(pair.second);
    }
    return result;
}

// 检查Surface是否存在
bool JJSurfaceManager::hasSurface(const std::string& surfaceId) const {
    return surfaces_.find(surfaceId) != surfaces_.end();
}

// 获取Surface使用的组件目录ID
std::string JJSurfaceManager::getCatalogId(const std::string& surfaceId) const {
    if (!hasSurface(surfaceId)) {
        return "";
    }
    return surfaces_.at(surfaceId).catalogId;
}

// 获取所有Surface的ID列表
std::vector<std::string> JJSurfaceManager::getAllSurfaceIds() const {
    std::vector<std::string> result;
    result.reserve(surfaces_.size());
    for (const auto& pair : surfaces_) {
        result.push_back(pair.first);
    }
    return result;
}

// 获取Surface中注册的组件数量
size_t JJSurfaceManager::getComponentCount(const std::string& surfaceId) const {
    if (!hasSurface(surfaceId)) {
        return 0;
    }
    return surfaces_.at(surfaceId).a2uiToHandle.size();
}

} // namespace jaether
