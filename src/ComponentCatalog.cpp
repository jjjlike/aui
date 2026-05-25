// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// ComponentCatalog.cpp
// 组件目录注册表模块 - 管理A2UI组件类型到jaether内部类型的映射
//
// 功能:
// - 注册A2UI组件类型与jaether类型的映射关系
// - 提供组件类型查询、属性校验
// - 内置jaether基本组件目录（jaether-basic-catalog）
// - 支持自定义组件类型扩展

#include "aether/ComponentCatalog.h"
#include <algorithm>

namespace jaether {

// 构造函数：初始化内置的jaether基本组件目录
// 注册所有第一阶段支持的A2UI组件类型
JJComponentCatalog::JJComponentCatalog() {
    // --- 布局组件 ---
    
    // Row组件：水平弹性布局容器
    registerType({
        "Row",
        JComponentType::Container,
        true,   // isLayout
        {},     // requiredProps: 无必填属性
        {"children", "justify", "align", "distribution", "alignment", "weight"}
    });

    // Column组件：垂直弹性布局容器
    registerType({
        "Column",
        JComponentType::Container,
        true,   // isLayout
        {},     // requiredProps: 无必填属性
        {"children", "justify", "align", "distribution", "alignment", "weight"}
    });

    // --- 显示组件 ---
    
    // Text组件：文本显示
    registerType({
        "Text",
        JComponentType::Text,
        false,  // isLayout
        {"text"},  // 必填属性：文本内容
        {"variant", "usageHint", "weight", "width", "height"}
    });

    // --- 交互组件 ---
    
    // Button组件：可点击按钮
    registerType({
        "Button",
        JComponentType::Button,
        false,  // isLayout
        {},     // requiredProps: 无必填属性（child可选）
        {"child", "action", "variant", "primary", "weight", "width", "height", "text"}
    });

    // TextField组件：文本输入框
    registerType({
        "TextField",
        JComponentType::Input,
        false,  // isLayout
        {},     // requiredProps: 无必填属性
        {"label", "value", "text", "textFieldType", "weight", "width", "height"}
    });

    // --- 容器组件 ---
    
    // Card组件：卡片容器
    registerType({
        "Card",
        JComponentType::Container,
        false,  // isLayout（Card不是Flexbox布局容器，但渲染时需要特殊处理）
        {},     // requiredProps: 无必填属性
        {"child", "weight", "width", "height"}
    });

    // CheckBox组件：复选框
    registerType({
        "CheckBox",
        JComponentType::Button,
        false,  // isLayout
        {"label"},  // 必填属性：标签文本
        {"value", "weight", "width", "height"}
    });
}

// 注册一个新的组件类型到目录中
// 同时建立正向(A2UI名→信息)和反向(枚举→A2UI名)映射
void JJComponentCatalog::registerType(const ComponentTypeInfo& info) {
    catalog_[info.a2uiName] = info;
    reverseMap_[info.jaetherType] = info.a2uiName;
}

// 根据A2UI类型名称查询jaether组件类型枚举值
// 未找到时返回 JComponentType::Custom 表示未注册类型
JComponentType JJComponentCatalog::getType(const std::string& a2uiName) const {
    auto it = catalog_.find(a2uiName);
    if (it != catalog_.end()) {
        return it->second.jaetherType;
    }
    return JComponentType::Custom;
}

// 根据jaether组件类型枚举值查询A2UI类型名称
// 用于将内部组件导出为A2UI JSON格式
std::string JJComponentCatalog::getA2UIName(JComponentType type) const {
    auto it = reverseMap_.find(type);
    if (it != reverseMap_.end()) {
        return it->second;
    }
    return "";
}

// 检查指定属性是否对给定组件类型有效
// 遍历该组件类型的必填属性和可选属性列表进行匹配
bool JJComponentCatalog::isValidProperty(const std::string& a2uiName, 
                                          const std::string& propName) const {
    auto it = catalog_.find(a2uiName);
    if (it == catalog_.end()) {
        return false;  // 组件类型未注册
    }

    const auto& info = it->second;
    
    // 在必填属性列表中查找
    if (std::find(info.requiredProps.begin(), info.requiredProps.end(), propName) 
        != info.requiredProps.end()) {
        return true;
    }
    
    // 在可选属性列表中查找
    if (std::find(info.optionalProps.begin(), info.optionalProps.end(), propName) 
        != info.optionalProps.end()) {
        return true;
    }

    return false;
}

// 检查组件类型是否已注册
bool JJComponentCatalog::isRegistered(const std::string& a2uiName) const {
    return catalog_.find(a2uiName) != catalog_.end();
}

// 获取指定属性对应的jaether PropertyId
// 使用框架现有的 getPropertyIdFromName 函数进行名称→ID映射
JPropertyId JJComponentCatalog::getPropertyId(const std::string& a2uiName, 
                                               const std::string& propName) const {
    // 先校验属性和类型有效性
    if (!isValidProperty(a2uiName, propName)) {
        return JPropertyId::Unknown;
    }
    
    // 将A2UI属性名映射到jaether属性ID
    // A2UI属性名与jaether属性名使用相同的camelCase命名规范
    
    // 直接映射的属性
    if (propName == "text")       return JPropertyId::Text;
    if (propName == "width")      return JPropertyId::Width;
    if (propName == "height")     return JPropertyId::Height;
    if (propName == "weight")     return JPropertyId::FlexGrow;
    if (propName == "variant")    return JPropertyId::FontSize;
    if (propName == "usageHint")  return JPropertyId::FontSize;
    if (propName == "label")      return JPropertyId::Text;
    
    // 布局属性
    if (propName == "justify")      return JPropertyId::JJustifyContent;
    if (propName == "distribution") return JPropertyId::JJustifyContent;
    if (propName == "align")        return JPropertyId::JAlignItems;
    if (propName == "alignment")    return JPropertyId::JAlignItems;
    
    // 按钮外观属性
    if (propName == "primary")     return JPropertyId::BackgroundColor;
    
    // 未映射的属性返回Unknown
    return JPropertyId::Unknown;
}

// 获取组件类型的必填属性列表
std::vector<std::string> JJComponentCatalog::getRequiredProps(const std::string& a2uiName) const {
    auto it = catalog_.find(a2uiName);
    if (it != catalog_.end()) {
        return it->second.requiredProps;
    }
    return {};
}

// 获取所有布局容器组件类型名称列表
std::vector<std::string> JJComponentCatalog::getLayoutTypes() const {
    std::vector<std::string> result;
    for (const auto& pair : catalog_) {
        if (pair.second.isLayout) {
            result.push_back(pair.first);
        }
    }
    return result;
}

// 判断指定A2UI类型是否为布局容器组件
bool JJComponentCatalog::isLayoutType(const std::string& a2uiName) const {
    auto it = catalog_.find(a2uiName);
    if (it != catalog_.end()) {
        return it->second.isLayout;
    }
    return false;
}

} // namespace jaether
