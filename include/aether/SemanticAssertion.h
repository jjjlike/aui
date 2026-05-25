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

#include "aether/types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <variant>

namespace jaether {

/**
 * 语义查询结果结构
 * 
 * 包含查询执行的结果信息
 */
struct SemanticQueryResult {
    bool found = false;                    // 是否找到匹配组件
    JComponentHandle handle;                // 找到的组件句柄
    JPropertyId propertyId = JPropertyId::Unknown;  // 相关的属性ID
    std::string expectedValue;             // 期望值
    std::string actualValue;               // 实际值
    bool conditionMet = false;             // 条件是否满足
    std::string error;                     // 错误信息（如果有）
};

/**
 * 语义谓词枚举
 * 
 * 定义支持的断言类型
 */
enum class SemanticPredicate {
    IsEnabled,          // 组件是否启用
    IsDisabled,         // 组件是否禁用
    IsVisible,          // 组件是否可见
    IsHidden,           // 组件是否隐藏
    HasText,            // 组件是否具有特定文本
    HasProperty,        // 组件是否具有特定属性
    IsInState,          // 组件是否处于特定状态
    Equals,             // 属性值是否等于
    GreaterThan,        // 属性值是否大于
    LessThan,           // 属性值是否小于
    Contains,           // 属性值是否包含
    Matches             // 属性值是否匹配（正则）
};

/**
 * 解析后的语义查询结构
 * 
 * 包含解析后的查询信息
 */
struct ParsedSemanticQuery {
    std::string selector;                 // 组件选择器（按文本或类型）
    std::string property;                 // 属性名
    SemanticPredicate predicate;          // 谓词类型
    std::string targetValue;              // 目标值（可选）
};

/**
 * 语义断言辅助类
 * 
 * 提供自然语言的UI断言功能
 * 用于测试和验证
 */
class SemanticAssertionHelper {
public:
    /**
     * 构造函数
     */
    SemanticAssertionHelper();
    
    /**
     * 析构函数
     */
    ~SemanticAssertionHelper();
    
    /**
     * 解析自然语言查询
     * @param naturalLanguageQuery 自然语言查询字符串
     * @return 解析后的查询结构
     */
    ParsedSemanticQuery parseQuery(const std::string& naturalLanguageQuery);
    
    /**
     * 执行语义查询
     * @param naturalLanguageQuery 自然语言查询
     * @param storage 组件存储
     * @return 查询结果
     */
    SemanticQueryResult executeQuery(
        const std::string& naturalLanguageQuery,
        class JComponentStorage& storage);
    
    /**
     * 断言组件启用
     * @param componentSelector 组件选择器
     * @param storage 组件存储
     * @return 断言是否通过
     */
    bool assertComponentEnabled(const std::string& componentSelector, class JComponentStorage& storage);
    
    /**
     * 断言组件禁用
     * @param componentSelector 组件选择器
     * @param storage 组件存储
     * @return 断言是否通过
     */
    bool assertComponentDisabled(const std::string& componentSelector, class JComponentStorage& storage);
    
    /**
     * 断言组件可见
     * @param componentSelector 组件选择器
     * @param storage 组件存储
     * @return 断言是否通过
     */
    bool assertComponentVisible(const std::string& componentSelector, class JComponentStorage& storage);
    
    /**
     * 断言组件具有特定文本
     * @param componentSelector 组件选择器
     * @param expectedText 期望的文本
     * @param storage 组件存储
     * @return 断言是否通过
     */
    bool assertComponentHasText(const std::string& componentSelector, const std::string& expectedText, class JComponentStorage& storage);
    
    /**
     * 注册自定义谓词
     * @param name 谓词名称
     * @param handler 处理函数
     */
    void registerPredicate(const std::string& name, std::function<bool(const std::string&, const std::string&)> handler);
    
private:
    /**
     * 通过选择器查找组件
     * @param selector 选择器
     * @param storage 组件存储
     * @return 匹配的组件列表
     */
    std::vector<JComponentHandle> findComponentsBySelector(const std::string& selector, class JComponentStorage& storage);
    
    /**
     * 通过文本查找组件
     * @param text 文本
     * @param storage 组件存储
     * @return 找到的组件句柄
     */
    JComponentHandle findComponentByText(const std::string& text, class JComponentStorage& storage);
    
    /**
     * 通过类型查找组件
     * @param typeName 类型名称
     * @param storage 组件存储
     * @return 找到的组件句柄
     */
    JComponentHandle findComponentByType(const std::string& typeName, class JComponentStorage& storage);
    
    /**
     * 属性名称到PropertyId的映射
     */
    std::unordered_map<std::string, JPropertyId> propertyNameMap;
    
    /**
     * 自定义谓词映射
     */
    std::unordered_map<std::string, std::function<bool(const std::string&, const std::string&)>> customPredicates;
    
    /**
     * 初始化默认映射
     */
    void initDefaultMappings();
};

} // namespace jaether
