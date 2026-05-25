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
#include "JSONParser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace jaether {

/**
 * 数据模型与绑定类
 * 
 * 管理A2UI协议的数据模型，支持路径访问和属性绑定
 * 当数据模型变更时自动通知所有绑定组件更新属性值
 */
class JJDataModel {
public:
    /**
     * 属性绑定回调类型
     * 当数据模型变更时触发，用于更新组件属性
     * 参数: 组件句柄, 属性ID, 新值
     */
    using BindingCallback = std::function<void(JComponentHandle, JPropertyId, const JJSONValue&)>;

    /**
     * 构造函数
     */
    JJDataModel();

    /**
     * 设置属性绑定回调函数
     * @param callback 当绑定数据变更时调用的回调
     */
    void setBindingCallback(BindingCallback callback);

    /**
     * 应用A2UI数据模型更新消息
     * 支持完全替换和路径追加两种模式
     * @param surfaceId 目标Surface标识符
     * @param path 更新路径，如"user"或"/user/email"，空字符串表示根路径
     * @param contents 数据条目数组，每项包含key和值
     */
    void applyUpdate(const std::string& surfaceId, const std::string& path, const JJSONArray& contents);

    /**
     * 获取数据模型中指定路径的值
     * @param surfaceId Surface标识符
     * @param path 数据路径，如"/user/name"
     * @return 路径对应的JSON值，不存在返回null
     */
    JJSONValue getValue(const std::string& surfaceId, const std::string& path) const;

    /**
     * 设置数据模型中指定路径的值
     * 如果路径的中间节点不存在则自动创建
     * @param surfaceId Surface标识符
     * @param path 数据路径，如"/user/name"
     * @param value 要设置的值
     */
    void setValue(const std::string& surfaceId, const std::string& path, const JJSONValue& value);

    /**
     * 注册组件属性的数据绑定
     * 当path对应的数据变更时，自动更新handle组件的propId属性
     * @param surfaceId Surface标识符
     * @param handle 组件句柄
     * @param propId 要绑定的属性ID
     * @param dataPath 数据模型路径，如"/user/name"
     */
    void bindProperty(const std::string& surfaceId, JComponentHandle handle, 
                      JPropertyId propId, const std::string& dataPath);

    /**
     * 解除组件的属性绑定
     * @param surfaceId Surface标识符
     * @param handle 组件句柄
     * @param dataPath 数据模型路径
     */
    void unbindProperty(const std::string& surfaceId, JComponentHandle handle, 
                        const std::string& dataPath);

    /**
     * 通知所有绑定到指定路径的组件数据已变更
     * @param surfaceId Surface标识符
     * @param changedPath 变更的数据路径
     */
    void notifyBindings(const std::string& surfaceId, const std::string& changedPath);

    /**
     * 导出数据模型为A2UI JSON格式字符串
     * @param surfaceId Surface标识符
     * @return A2UI格式的JSON字符串
     */
    std::string exportJSON(const std::string& surfaceId) const;

    /**
     * 检查指定Surface是否有数据模型
     * @param surfaceId Surface标识符
     * @return 有数据模型返回true
     */
    bool hasDataModel(const std::string& surfaceId) const;

    /**
     * 清空指定Surface的数据模型
     * @param surfaceId Surface标识符
     */
    void clearDataModel(const std::string& surfaceId);

    /**
     * 返回指定路径是否存在
     * @param surfaceId Surface标识符
     * @param path 数据路径
     * @return 存在返回true
     */
    bool pathExists(const std::string& surfaceId, const std::string& path) const;

private:
    /**
     * 属性绑定记录结构体
     * 存储单个属性绑定的目标组件和属性ID
     */
    struct PropertyBinding {
        JComponentHandle handle;   // 绑定的目标组件句柄
        JPropertyId propId;        // 要更新的属性ID
    };

    /**
     * 将路径字符串解析为路径段数组
     * 例如 "/user/name" → ["user", "name"]
     * @param path 原始路径字符串
     * @return 路径段数组
     */
    std::vector<std::string> parsePath(const std::string& path) const;

    /**
     * 在数据模型中设置值（内部实现，支持路径遍历和自动创建中间节点）
     * @param surfaceId Surface标识符
     * @param pathSegments 路径段数组
     * @param value 要设置的值
     * @param depth 当前递归深度
     */
    void setValueAtPath(const std::string& surfaceId, 
                        const std::vector<std::string>& pathSegments, 
                        const JJSONValue& value);

    /**
     * 从数据模型中获取值（内部实现，支持路径遍历）
     * @param surfaceId Surface标识符
     * @param pathSegments 路径段数组
     * @return 路径对应的JSON值
     */
    JJSONValue getValueAtPath(const std::string& surfaceId,
                               const std::vector<std::string>& pathSegments) const;

    BindingCallback bindingCallback_;    // 属性绑定变更回调
    
    // surfaceId → 数据模型根对象（使用JJSONObject嵌套存储树形数据）
    std::unordered_map<std::string, JJSONObject> dataModels_;
    
    // surfaceId → (数据路径 → 绑定列表)
    std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<PropertyBinding>>> bindings_;
};

} // namespace jaether
