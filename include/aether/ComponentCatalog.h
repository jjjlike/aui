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
#include <string>
#include <vector>
#include <unordered_map>

namespace jaether {

/**
 * 组件目录注册表类
 * 
 * 管理A2UI组件类型到jaether内部类型的映射关系
 * 提供组件类型查询、属性校验、默认值填充等功能
 * 是A2UI协议层的核心基础设施
 */
class JJComponentCatalog {
public:
    /**
     * 组件类型元信息结构体
     * 
     * 描述一个A2UI组件类型的所有元数据
     */
    struct ComponentTypeInfo {
        std::string a2uiName;                    // A2UI类型名称，如"Text"、"Button"
        JComponentType jaetherType;              // jaether内部组件类型枚举值
        bool isLayout;                           // 是否为布局容器组件
        std::vector<std::string> requiredProps;  // 必填属性名称列表
        std::vector<std::string> optionalProps;  // 可选属性名称列表
    };

    /**
     * 构造函数
     * 初始化内置的jaether基本组件目录
     */
    JJComponentCatalog();

    /**
     * 注册一个新的组件类型
     * @param info 组件类型元信息
     */
    void registerType(const ComponentTypeInfo& info);

    /**
     * 根据A2UI类型名称查询jaether组件类型
     * @param a2uiName A2UI类型名称，如"Text"
     * @return jaether组件类型枚举值，未找到返回Custom
     */
    JComponentType getType(const std::string& a2uiName) const;

    /**
     * 根据jaether组件类型查询A2UI类型名称
     * @param type jaether组件类型枚举值
     * @return A2UI类型名称字符串，未找到返回空字符串
     */
    std::string getA2UIName(JComponentType type) const;

    /**
     * 检查指定属性是否对给定组件类型有效
     * @param a2uiName A2UI类型名称
     * @param propName A2UI属性名称
     * @return 如果该属性是该类型的有效属性则返回true
     */
    bool isValidProperty(const std::string& a2uiName, const std::string& propName) const;

    /**
     * 检查给定组件类型是否已注册
     * @param a2uiName A2UI类型名称
     * @return 如果已注册则返回true
     */
    bool isRegistered(const std::string& a2uiName) const;

    /**
     * 获取指定属性的jaether PropertyId
     * @param a2uiName A2UI类型名称
     * @param propName A2UI属性名称
     * @return jaether属性ID，未找到返回Unknown
     */
    JPropertyId getPropertyId(const std::string& a2uiName, const std::string& propName) const;

    /**
     * 获取组件类型的必填属性列表
     * @param a2uiName A2UI类型名称
     * @return 必填属性名称列表
     */
    std::vector<std::string> getRequiredProps(const std::string& a2uiName) const;

    /**
     * 获取布局容器组件类型名称列表
     * @return 所有布局组件类型的A2UI名称
     */
    std::vector<std::string> getLayoutTypes() const;

    /**
     * 判断是否为布局容器组件
     * @param a2uiName A2UI类型名称
     * @return 如果是布局容器组件则返回true
     */
    bool isLayoutType(const std::string& a2uiName) const;

private:
    /**
     * A2UI类型名 → 组件类型元信息 的映射表
     */
    std::unordered_map<std::string, ComponentTypeInfo> catalog_;

    /**
     * jaether组件类型枚举值 → A2UI类型名 的反向映射表
     */
    std::unordered_map<JComponentType, std::string> reverseMap_;
};

} // namespace jaether
