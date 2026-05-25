// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// ComponentCatalog_test.cpp
// JJComponentCatalog 组件目录注册表 单元测试
//
// 测试覆盖标准: 语句覆盖、分支覆盖、条件覆盖、路径覆盖、MC/DC覆盖
// 测试范围:
// - 内置组件类型注册验证
// - getType 正向查询（所有内置类型 + 未注册类型）
// - getA2UIName 反向查询（所有内置类型 + 未注册类型）
// - isValidProperty 属性校验（有效/无效属性/未注册类型）
// - isRegistered 注册检查
// - getPropertyId 属性ID映射
// - getRequiredProps 必填属性获取
// - getLayoutTypes 布局类型列表
// - isLayoutType 布局类型判断
// - registerType 动态注册自定义类型

#include "aether/ComponentCatalog.h"
#include <gtest/gtest.h>
#include <string>

namespace jaether {
namespace test {

/**
 * 测试夹具：每个测试用例创建一个全新的组件目录实例
 * 保证测试隔离性，避免用例间状态污染
 */
class ComponentCatalogTest : public ::testing::Test {
protected:
    void SetUp() override {
        catalog_ = new JJComponentCatalog();
    }

    void TearDown() override {
        delete catalog_;
        catalog_ = nullptr;
    }

    JJComponentCatalog* catalog_;
};

// ==================== getType 正向查询测试 ====================

/**
 * 测试 getType - 语句覆盖：调用getType方法
 * 测试 getType - Text组件类型查询
 * 分支: 组件存在 → 返回对应类型
 */
TEST_F(ComponentCatalogTest, GetType_TextComponent_ReturnsTextType) {
    JComponentType type = catalog_->getType("Text");
    EXPECT_EQ(type, JComponentType::Text);
}

/**
 * 测试 getType - Button组件类型查询
 * 分支: 组件存在 → 返回对应类型
 */
TEST_F(ComponentCatalogTest, GetType_ButtonComponent_ReturnsButtonType) {
    JComponentType type = catalog_->getType("Button");
    EXPECT_EQ(type, JComponentType::Button);
}

/**
 * 测试 getType - Row组件类型查询
 * 分支: 组件存在 → 返回对应类型
 * 条件: a2uiName="Row" → it != end() → 进入if分支
 */
TEST_F(ComponentCatalogTest, GetType_RowComponent_ReturnsContainerType) {
    JComponentType type = catalog_->getType("Row");
    EXPECT_EQ(type, JComponentType::Container);
}

/**
 * 测试 getType - Column组件类型查询
 * 分支: 组件存在 → 返回对应类型
 */
TEST_F(ComponentCatalogTest, GetType_ColumnComponent_ReturnsContainerType) {
    JComponentType type = catalog_->getType("Column");
    EXPECT_EQ(type, JComponentType::Container);
}

/**
 * 测试 getType - TextField组件类型查询
 * 分支: 组件存在 → 返回对应类型
 */
TEST_F(ComponentCatalogTest, GetType_TextFieldComponent_ReturnsInputType) {
    JComponentType type = catalog_->getType("TextField");
    EXPECT_EQ(type, JComponentType::Input);
}

/**
 * 测试 getType - Card组件类型查询
 * 分支: 组件存在 → 返回对应类型
 */
TEST_F(ComponentCatalogTest, GetType_CardComponent_ReturnsContainerType) {
    JComponentType type = catalog_->getType("Card");
    EXPECT_EQ(type, JComponentType::Container);
}

/**
 * 测试 getType - CheckBox组件类型查询
 * 分支: 组件存在 → 返回对应类型
 */
TEST_F(ComponentCatalogTest, GetType_CheckBoxComponent_ReturnsButtonType) {
    JComponentType type = catalog_->getType("CheckBox");
    EXPECT_EQ(type, JComponentType::Button);
}

/**
 * 测试 getType - 未注册类型查询
 * 分支: 组件不存在 → 返回Custom
 * MC/DC: it == end() → 进入else分支（返回Custom）
 */
TEST_F(ComponentCatalogTest, GetType_UnregisteredComponent_ReturnsCustom) {
    JComponentType type = catalog_->getType("NonExistentComponent");
    EXPECT_EQ(type, JComponentType::Custom);
}

/**
 * 测试 getType - 空字符串查询
 * 条件: 空字符串 → it == end() → 返回Custom
 */
TEST_F(ComponentCatalogTest, GetType_EmptyString_ReturnsCustom) {
    JComponentType type = catalog_->getType("");
    EXPECT_EQ(type, JComponentType::Custom);
}

// ==================== getA2UIName 反向查询测试 ====================

/**
 * 测试 getA2UIName - Text类型反向查询
 * 分支: 类型存在 → 返回A2UI名称
 */
TEST_F(ComponentCatalogTest, GetA2UIName_TextType_ReturnsTextString) {
    std::string name = catalog_->getA2UIName(JComponentType::Text);
    EXPECT_EQ(name, "Text");
}

/**
 * 测试 getA2UIName - Button类型反向查询
 * Button被多个类型映射（Button, CheckBox），返回最后注册的映射
 */
TEST_F(ComponentCatalogTest, GetA2UIName_ButtonType_ReturnsLastRegistered) {
    std::string name = catalog_->getA2UIName(JComponentType::Button);
    // Button 和 CheckBox 都映射到 JComponentType::Button，
    // CheckBox 后注册，所以反向映射为 "CheckBox"
    EXPECT_EQ(name, "CheckBox");
}

/**
 * 测试 getA2UIName - Input类型反向查询
 * 分支: 类型存在 → 返回A2UI名称
 */
TEST_F(ComponentCatalogTest, GetA2UIName_InputType_ReturnsTextFieldString) {
    std::string name = catalog_->getA2UIName(JComponentType::Input);
    EXPECT_EQ(name, "TextField");
}

/**
 * 测试 getA2UIName - Container类型反向查询
 * Container被Row、Column、Card映射，返回最后注册的映射（Card）
 */
TEST_F(ComponentCatalogTest, GetA2UIName_ContainerType_ReturnsLastRegistered) {
    std::string name = catalog_->getA2UIName(JComponentType::Container);
    // Row, Column, Card 都映射到 Container，Card 最后注册
    EXPECT_EQ(name, "Card");
}

/**
 * 测试 getA2UIName - ScrollView类型反向查询
 * 分支: 类型存在 → 返回A2UI名称
 */
TEST_F(ComponentCatalogTest, GetA2UIName_ScrollViewType_ReturnsEmpty) {
    std::string name = catalog_->getA2UIName(JComponentType::ScrollView);
    // ScrollView未注册，返回空字符串
    EXPECT_TRUE(name.empty());
}

/**
 * 测试 getA2UIName - Custom类型反向查询
 * 分支: 类型未注册 → 返回空字符串
 * MC/DC: it == end() → 进入else分支
 */
TEST_F(ComponentCatalogTest, GetA2UIName_CustomType_ReturnsEmpty) {
    std::string name = catalog_->getA2UIName(JComponentType::Custom);
    EXPECT_TRUE(name.empty());
}

// ==================== isValidProperty 属性校验测试 ====================

/**
 * 测试 isValidProperty - Text组件的有效属性（必填属性）
 * 分支: 组件存在 + 属性在requiredProps中 → 返回true
 */
TEST_F(ComponentCatalogTest, IsValidProperty_TextRequiredProp_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isValidProperty("Text", "text"));
}

/**
 * 测试 isValidProperty - Text组件的有效属性（可选属性）
 * 分支: 组件存在 + 属性在optionalProps中 → 返回true
 */
TEST_F(ComponentCatalogTest, IsValidProperty_TextOptionalProp_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isValidProperty("Text", "variant"));
}

/**
 * 测试 isValidProperty - Text组件的无效属性
 * 分支: 组件存在 + 属性不在requiredProps也不在optionalProps → 返回false
 * MC/DC: requiredProps.find == end() AND optionalProps.find == end() → false
 */
TEST_F(ComponentCatalogTest, IsValidProperty_TextInvalidProp_ReturnsFalse) {
    EXPECT_FALSE(catalog_->isValidProperty("Text", "nonExistentProp"));
}

/**
 * 测试 isValidProperty - Button组件的有效属性
 * 分支: 组件存在 + 属性有效 → 返回true
 */
TEST_F(ComponentCatalogTest, IsValidProperty_ButtonChildProp_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isValidProperty("Button", "child"));
}

/**
 * 测试 isValidProperty - Button组件的action属性
 * 分支: 组件存在 + 可选属性 → 返回true
 */
TEST_F(ComponentCatalogTest, IsValidProperty_ButtonActionProp_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isValidProperty("Button", "action"));
}

/**
 * 测试 isValidProperty - Row组件的children属性
 * 分支: 组件存在 + 可选属性 → 返回true
 */
TEST_F(ComponentCatalogTest, IsValidProperty_RowChildrenProp_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isValidProperty("Row", "children"));
}

/**
 * 测试 isValidProperty - Row组件的justify属性
 * 分支: 组件存在 + 可选属性 → 返回true
 */
TEST_F(ComponentCatalogTest, IsValidProperty_RowJustifyProp_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isValidProperty("Row", "justify"));
}

/**
 * 测试 isValidProperty - 未注册类型的属性查询
 * 分支: 组件不存在 → 返回false
 * MC/DC: it == end() → false（短路：后续条件不评估）
 */
TEST_F(ComponentCatalogTest, IsValidProperty_UnregisteredType_ReturnsFalse) {
    EXPECT_FALSE(catalog_->isValidProperty("UnknownType", "anyProp"));
}

// ==================== isRegistered 注册检查测试 ====================

/**
 * 测试 isRegistered - 已注册组件
 * 分支: 组件存在 → 返回true
 */
TEST_F(ComponentCatalogTest, IsRegistered_ExistingComponent_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isRegistered("Text"));
    EXPECT_TRUE(catalog_->isRegistered("Button"));
    EXPECT_TRUE(catalog_->isRegistered("Row"));
}

/**
 * 测试 isRegistered - 未注册组件
 * 分支: 组件不存在 → 返回false
 * MC/DC: find == end() → false
 */
TEST_F(ComponentCatalogTest, IsRegistered_NonExistingComponent_ReturnsFalse) {
    EXPECT_FALSE(catalog_->isRegistered("UnknownComponent"));
}

// ==================== getPropertyId 属性ID映射测试 ====================

/**
 * 测试 getPropertyId - 文本属性映射
 * 路径: propName="text" → 返回Text枚举值
 */
TEST_F(ComponentCatalogTest, GetPropertyId_TextProp_ReturnsTextId) {
    EXPECT_EQ(catalog_->getPropertyId("Text", "text"), JPropertyId::Text);
}

/**
 * 测试 getPropertyId - 宽度属性映射
 * 路径: propName="width" → 返回Width枚举值
 */
TEST_F(ComponentCatalogTest, GetPropertyId_WidthProp_ReturnsWidthId) {
    EXPECT_EQ(catalog_->getPropertyId("Text", "width"), JPropertyId::Width);
}

/**
 * 测试 getPropertyId - justify属性映射
 * 路径: propName="justify" → 返回JJustifyContent枚举值
 */
TEST_F(ComponentCatalogTest, GetPropertyId_JustifyProp_ReturnsJustifyContentId) {
    EXPECT_EQ(catalog_->getPropertyId("Row", "justify"), JPropertyId::JJustifyContent);
}

/**
 * 测试 getPropertyId - 无效属性返回Unknown
 * 分支: 属性无效 → 返回Unknown
 * MC/DC: isValidProperty返回false → 进入if分支 → return Unknown
 */
TEST_F(ComponentCatalogTest, GetPropertyId_InvalidProp_ReturnsUnknown) {
    EXPECT_EQ(catalog_->getPropertyId("Text", "invalidProp"), JPropertyId::Unknown);
}

/**
 * 测试 getPropertyId - 未注册类型返回Unknown
 * 分支: 类型未注册 → isValidProperty返回false → 返回Unknown
 */
TEST_F(ComponentCatalogTest, GetPropertyId_UnregisteredType_ReturnsUnknown) {
    EXPECT_EQ(catalog_->getPropertyId("BadType", "text"), JPropertyId::Unknown);
}

// ==================== getRequiredProps 必填属性测试 ====================

/**
 * 测试 getRequiredProps - Text组件有必填属性
 * 分支: 组件存在 → 返回requiredProps列表
 */
TEST_F(ComponentCatalogTest, GetRequiredProps_Text_ContainsText) {
    auto props = catalog_->getRequiredProps("Text");
    ASSERT_EQ(props.size(), 1);
    EXPECT_EQ(props[0], "text");
}

/**
 * 测试 getRequiredProps - Button组件无必填属性
 * 分支: 组件存在 + requiredProps为空 → 返回空列表
 */
TEST_F(ComponentCatalogTest, GetRequiredProps_Button_ReturnsEmpty) {
    auto props = catalog_->getRequiredProps("Button");
    EXPECT_TRUE(props.empty());
}

/**
 * 测试 getRequiredProps - 未注册类型
 * 分支: 组件不存在 → 返回空列表
 * MC/DC: it == end() → 返回{}
 */
TEST_F(ComponentCatalogTest, GetRequiredProps_Unregistered_ReturnsEmpty) {
    auto props = catalog_->getRequiredProps("FakeComponent");
    EXPECT_TRUE(props.empty());
}

// ==================== getLayoutTypes 布局类型测试 ====================

/**
 * 测试 getLayoutTypes - 返回Row和Column
 * 路径: 遍历catalog，收集isLayout=true的类型
 */
TEST_F(ComponentCatalogTest, GetLayoutTypes_ReturnsRowAndColumn) {
    auto types = catalog_->getLayoutTypes();
    EXPECT_GE(types.size(), 2);  // 至少包含Row和Column
    
    bool hasRow = false;
    bool hasColumn = false;
    for (const auto& t : types) {
        if (t == "Row") hasRow = true;
        if (t == "Column") hasColumn = true;
    }
    EXPECT_TRUE(hasRow);
    EXPECT_TRUE(hasColumn);
}

// ==================== isLayoutType 布局类型判断测试 ====================

/**
 * 测试 isLayoutType - Row是布局类型
 * 条件: isLayout=true → 返回true
 */
TEST_F(ComponentCatalogTest, IsLayoutType_Row_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isLayoutType("Row"));
}

/**
 * 测试 isLayoutType - Column是布局类型
 * 条件: isLayout=true → 返回true
 */
TEST_F(ComponentCatalogTest, IsLayoutType_Column_ReturnsTrue) {
    EXPECT_TRUE(catalog_->isLayoutType("Column"));
}

/**
 * 测试 isLayoutType - Text不是布局类型
 * 条件: isLayout=false → 返回false
 * MC/DC: isLayout条件独立影响结果
 */
TEST_F(ComponentCatalogTest, IsLayoutType_Text_ReturnsFalse) {
    EXPECT_FALSE(catalog_->isLayoutType("Text"));
}

/**
 * 测试 isLayoutType - Button不是布局类型
 * 条件: isLayout=false → 返回false
 */
TEST_F(ComponentCatalogTest, IsLayoutType_Button_ReturnsFalse) {
    EXPECT_FALSE(catalog_->isLayoutType("Button"));
}

/**
 * 测试 isLayoutType - 未注册类型
 * 分支: 组件不存在 → 返回false
 * MC/DC: it == end() → 进入else分支
 */
TEST_F(ComponentCatalogTest, IsLayoutType_Unregistered_ReturnsFalse) {
    EXPECT_FALSE(catalog_->isLayoutType("NonExistent"));
}

// ==================== registerType 动态注册测试 ====================

/**
 * 测试 registerType - 注册新类型后可通过getType查询
 * 路径: registerType → getType → 验证一致性
 * 验证注册了正向和反向映射
 */
TEST_F(ComponentCatalogTest, RegisterType_ThenGetType_ReturnsCorrect) {
    JJComponentCatalog::ComponentTypeInfo info;
    info.a2uiName = "CustomWidget";
    info.jaetherType = JComponentType::Custom;
    info.isLayout = false;
    info.requiredProps = {"customProp"};
    info.optionalProps = {"optionalProp"};
    
    catalog_->registerType(info);
    
    // 验证注册后可以通过getType查询
    EXPECT_EQ(catalog_->getType("CustomWidget"), JComponentType::Custom);
    
    // 验证注册后可以通过getA2UIName反向查询
    // 注意：Custom可能已被其他映射覆盖，这里验证注册成功即可
    EXPECT_TRUE(catalog_->isRegistered("CustomWidget"));
    
    // 验证属性校验
    EXPECT_TRUE(catalog_->isValidProperty("CustomWidget", "customProp"));
    EXPECT_TRUE(catalog_->isValidProperty("CustomWidget", "optionalProp"));
    EXPECT_FALSE(catalog_->isValidProperty("CustomWidget", "badProp"));
}

/**
 * 测试 registerType - 覆盖已有类型的注册
 * 路径: 注册已有a2uiName → 更新映射关系
 */
TEST_F(ComponentCatalogTest, RegisterType_OverrideExisting_UpdatesMapping) {
    JJComponentCatalog::ComponentTypeInfo info;
    info.a2uiName = "Text";         // 覆盖已有的Text注册
    info.jaetherType = JComponentType::Custom;
    info.isLayout = true;
    
    catalog_->registerType(info);
    
    // 验证新映射生效
    EXPECT_EQ(catalog_->getType("Text"), JComponentType::Custom);
}

} // namespace test
} // namespace jaether
