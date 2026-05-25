#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace jaether {

/**
 * Surface生命周期管理器类
 * 
 * 管理A2UI Surface的创建、销毁和组件ID映射
 * 维护A2UI字符串ID与jaether JComponentHandle之间的双向映射关系
 * 支持多Surface并存，每个Surface拥有独立的组件命名空间
 */
class JJSurfaceManager {
public:
    /**
     * Surface数据结构
     * 
     * 存储单个Surface的所有元信息
     */
    struct Surface {
        std::string id;                                   // Surface唯一标识符
        std::string catalogId;                            // 使用的组件目录ID
        JComponentHandle rootHandle;                      // 根组件句柄
        std::unordered_map<std::string, JComponentHandle> a2uiToHandle;  // A2UI ID → jaether句柄
        std::unordered_map<int32_t, std::string> handleToA2UI;           // jaether句柄索引 → A2UI ID
    };

    /**
     * 构造函数
     * @param storage 组件存储引用，用于验证和操作组件
     */
    explicit JJSurfaceManager(JComponentStorage& storage);

    /**
     * 创建新的Surface
     * @param surfaceId 唯一Surface标识符，不能与已有Surface重复
     * @param catalogId 使用的组件目录ID，默认为"jaether-basic"
     * @return 创建成功返回true，surfaceId已存在返回false
     */
    bool createSurface(const std::string& surfaceId, const std::string& catalogId = "jaether-basic");

    /**
     * 删除Surface及其所有关联的组件映射
     * 注意：此方法不销毁组件本身，只清理ID映射关系
     * @param surfaceId 要删除的Surface标识符
     */
    void deleteSurface(const std::string& surfaceId);

    /**
     * 注册组件的A2UI ID映射
     * 建立A2UI字符串ID与jaether组件句柄之间的双向关联
     * @param surfaceId Surface标识符
     * @param a2uiId A2UI组件ID字符串
     * @param handle jaether组件句柄
     */
    void registerComponent(const std::string& surfaceId, const std::string& a2uiId, JComponentHandle handle);

    /**
     * 注销组件映射
     * 移除A2UI ID与jaether组件句柄之间的关联关系
     * @param surfaceId Surface标识符
     * @param a2uiId A2UI组件ID字符串
     */
    void unregisterComponent(const std::string& surfaceId, const std::string& a2uiId);

    /**
     * 通过A2UI ID查找对应的jaether组件句柄
     * @param surfaceId Surface标识符
     * @param a2uiId A2UI组件ID字符串
     * @return 找到的组件句柄，未找到返回无效句柄
     */
    JComponentHandle findComponent(const std::string& surfaceId, const std::string& a2uiId) const;

    /**
     * 通过jaether组件句柄查找对应的A2UI ID
     * @param surfaceId Surface标识符
     * @param handle jaether组件句柄
     * @return A2UI组件ID字符串，未找到返回空字符串
     */
    std::string findA2UIId(const std::string& surfaceId, JComponentHandle handle) const;

    /**
     * 获取Surface的根组件句柄
     * @param surfaceId Surface标识符
     * @return 根组件句柄，Surface不存在返回无效句柄
     */
    JComponentHandle getRootComponent(const std::string& surfaceId) const;

    /**
     * 设置Surface的根组件句柄
     * @param surfaceId Surface标识符
     * @param handle 根组件句柄
     */
    void setRootComponent(const std::string& surfaceId, JComponentHandle handle);

    /**
     * 获取Surface中所有组件的A2UI ID列表
     * @param surfaceId Surface标识符
     * @return A2UI组件ID字符串列表
     */
    std::vector<std::string> getAllComponentIds(const std::string& surfaceId) const;

    /**
     * 获取Surface中所有组件的jaether句柄列表
     * @param surfaceId Surface标识符
     * @return 组件句柄列表
     */
    std::vector<JComponentHandle> getAllComponentHandles(const std::string& surfaceId) const;

    /**
     * 检查Surface是否存在
     * @param surfaceId Surface标识符
     * @return 存在返回true
     */
    bool hasSurface(const std::string& surfaceId) const;

    /**
     * 获取Surface使用的组件目录ID
     * @param surfaceId Surface标识符
     * @return 目录ID字符串
     */
    std::string getCatalogId(const std::string& surfaceId) const;

    /**
     * 获取所有Surface的ID列表
     * @return Surface ID字符串列表
     */
    std::vector<std::string> getAllSurfaceIds() const;

    /**
     * 获取Surface中注册的组件数量
     * @param surfaceId Surface标识符
     * @return 组件数量，Surface不存在返回0
     */
    size_t getComponentCount(const std::string& surfaceId) const;

private:
    JComponentStorage& storage_;                            // 组件存储引用
    std::unordered_map<std::string, Surface> surfaces_;     // Surface ID → Surface数据
};

} // namespace jaether
