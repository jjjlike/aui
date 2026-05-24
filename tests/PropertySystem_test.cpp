/**
 * 属性系统模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 PropertyValue 类的类型存储和转换
 * - 测试 PropertyBlock 类的属性管理
 * - 测试 PropertyId 的查找和名称转换
 * - 测试布局相关属性的判断
 * - 测试用例覆盖：所有数据类型、边界情况
 */

#include "aether/ComponentStorage.h"
#include <gtest/gtest.h>
#include <string>

namespace aether {
namespace test {

/**
 * 测试用例 1：整数类型 PropertyValue（正常逻辑测试）
 * 测试目标：验证整数类型的属性值能正确存储和读取
 */
TEST(PropertySystemTest, PropertyValueInt) {
    // 创建一个值为 42 的整数属性
    PropertyValue val(42);

    // 验证类型判断正确
    EXPECT_TRUE(val.is<int>());
    // 验证值读取正确
    EXPECT_EQ(val.get<int>(), 42);

    // 测试负整数
    PropertyValue neg(-123);
    EXPECT_EQ(neg.get<int>(), -123);

    // 测试零值
    PropertyValue zero(0);
    EXPECT_EQ(zero.get<int>(), 0);
}

/**
 * 测试用例 2：浮点数类型 PropertyValue（正常逻辑测试）
 * 测试目标：验证浮点数类型的属性值能正确存储和读取
 */
TEST(PropertySystemTest, PropertyValueFloat) {
    // 创建一个值为 3.14 的浮点数属性
    PropertyValue val(3.14f);

    // 验证类型判断正确
    EXPECT_TRUE(val.is<float>());
    // 验证值读取正确（用浮点比较）
    EXPECT_FLOAT_EQ(val.get<float>(), 3.14f);

    // 测试负浮点数
    PropertyValue neg(-2.5f);
    EXPECT_FLOAT_EQ(neg.get<float>(), -2.5f);
}

/**
 * 测试用例 3：布尔类型 PropertyValue（正常逻辑测试）
 * 测试目标：验证布尔类型的属性值能正确存储和读取
 */
TEST(PropertySystemTest, PropertyValueBool) {
    // 创建一个 true 值的布尔属性
    PropertyValue trueVal(true);
    EXPECT_TRUE(trueVal.is<bool>());
    EXPECT_TRUE(trueVal.get<bool>());

    // 创建一个 false 值的布尔属性
    PropertyValue falseVal(false);
    EXPECT_TRUE(falseVal.is<bool>());
    EXPECT_FALSE(falseVal.get<bool>());
}

/**
 * 测试用例 4：字符串类型 PropertyValue（正常逻辑测试）
 * 测试目标：验证字符串类型的属性值能正确存储和读取
 */
TEST(PropertySystemTest, PropertyValueString) {
    // 创建一个值为 "hello" 的字符串属性
    PropertyValue val(std::string("hello"));

    // 验证类型判断正确
    EXPECT_TRUE(val.is<std::string>());
    // 验证值读取正确
    EXPECT_EQ(val.get<std::string>(), "hello");

    // 测试空字符串
    PropertyValue empty(std::string(""));
    EXPECT_EQ(empty.get<std::string>(), "");
}

/**
 * 测试用例 5：颜色类型 PropertyValue（正常逻辑测试）
 * 测试目标：验证颜色类型的属性值能正确存储和读取
 */
TEST(PropertySystemTest, PropertyValueColor) {
    // 创建一个颜色（RGBA：255, 128, 64, 255）
    Color c(static_cast<uint8_t>(255), static_cast<uint8_t>(128), static_cast<uint8_t>(64), static_cast<uint8_t>(255));
    PropertyValue val(c);

    // 验证类型判断正确
    EXPECT_TRUE(val.is<Color>());
    // 验证红色通道值
    EXPECT_EQ(val.get<Color>().r, 255);
    // 验证绿色通道值
    EXPECT_EQ(val.get<Color>().g, 128);
    // 验证蓝色通道值
    EXPECT_EQ(val.get<Color>().b, 64);
}

/**
 * 测试用例 6：空的 PropertyValue（Monostate）（边界测试）
 * 测试目标：验证默认构造的 PropertyValue（空值）的行为
 */
TEST(PropertySystemTest, PropertyValueMonostate) {
    // 创建一个默认构造的空 PropertyValue
    PropertyValue empty;

    // 验证它不匹配任何类型
    EXPECT_FALSE(empty.is<int>());
    EXPECT_FALSE(empty.is<float>());
    EXPECT_FALSE(empty.is<bool>());
    EXPECT_FALSE(empty.is<std::string>());
    EXPECT_FALSE(empty.is<Color>());
}

/**
 * 测试用例 7：PropertyValue 类型转换（分支测试）
 * 测试目标：验证不同类型之间的转换逻辑（本测试展示显式转换）
 */
TEST(PropertySystemTest, PropertyValueConversion) {
    PropertyValue intVal(100);
    EXPECT_TRUE(intVal.is<int>());

    // 显式转换为 float 类型
    PropertyValue floatVal(static_cast<float>(intVal.get<int>()));
    EXPECT_TRUE(floatVal.is<float>());
    EXPECT_FLOAT_EQ(floatVal.get<float>(), 100.0f);

    // 显式转换为字符串类型
    PropertyValue strVal(std::to_string(intVal.get<int>()));
    EXPECT_TRUE(strVal.is<std::string>());
}

/**
 * 测试用例 8：PropertyBlock 设置和获取属性（正常逻辑测试）
 * 测试目标：验证 PropertyBlock 的 setProperty 和 getProperty 功能
 */
TEST(PropertySystemTest, PropertyBlockSetAndGet) {
    PropertyBlock block;

    // 设置多个属性：宽度、高度、文本
    block.setProperty(PropertyId::Width, PropertyValue(100));
    block.setProperty(PropertyId::Height, PropertyValue(50));
    block.setProperty(PropertyId::Text, PropertyValue(std::string("Test")));

    // 验证 hasProperty 能正确判断属性是否存在
    EXPECT_TRUE(block.hasProperty(PropertyId::Width));
    EXPECT_TRUE(block.hasProperty(PropertyId::Height));
    EXPECT_FALSE(block.hasProperty(PropertyId::MarginLeft));  // 验证不存在的属性

    // 验证能正确获取宽度属性
    auto* width = block.getProperty(PropertyId::Width);
    ASSERT_NE(width, nullptr);
    EXPECT_TRUE(width->is<int>());
    EXPECT_EQ(width->get<int>(), 100);

    // 验证能正确获取文本属性
    auto* text = block.getProperty(PropertyId::Text);
    ASSERT_NE(text, nullptr);
    EXPECT_TRUE(text->is<std::string>());
    EXPECT_EQ(text->get<std::string>(), "Test");
}

/**
 * 测试用例 9：PropertyBlock 覆盖属性（分支测试）
 * 测试目标：验证对已存在的属性重新设置时，能正确覆盖旧值
 */
TEST(PropertySystemTest, PropertyBlockOverwrite) {
    PropertyBlock block;

    // 先设置宽度为 100
    block.setProperty(PropertyId::Width, PropertyValue(100));

    // 验证初始值正确
    auto* first = block.getProperty(PropertyId::Width);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->get<int>(), 100);

    // 覆盖为新值 200
    block.setProperty(PropertyId::Width, PropertyValue(200));

    // 验证新值正确
    auto* second = block.getProperty(PropertyId::Width);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->get<int>(), 200);
}

/**
 * 测试用例 10：PropertyBlock 清空属性（正常逻辑测试）
 * 测试目标：验证 clearProperty 能正确移除一个属性
 */
TEST(PropertySystemTest, PropertyBlockClear) {
    PropertyBlock block;
    // 设置文本属性
    block.setProperty(PropertyId::Text, PropertyValue(std::string("test")));
    // 验证属性存在
    EXPECT_TRUE(block.hasProperty(PropertyId::Text));

    // 清空文本属性
    block.clearProperty(PropertyId::Text);
    // 验证属性已不存在
    EXPECT_FALSE(block.hasProperty(PropertyId::Text));
}

/**
 * 测试用例 11：PropertyId 通过名称查找（正常逻辑测试）
 * 测试目标：验证 getPropertyIdFromName 能通过属性名找到对应的枚举值
 */
TEST(PropertySystemTest, PropertyIdLookup) {
    // 验证通过名称找到正确的枚举
    EXPECT_EQ(getPropertyIdFromName("width"), PropertyId::Width);
    EXPECT_EQ(getPropertyIdFromName("height"), PropertyId::Height);
    EXPECT_EQ(getPropertyIdFromName("text"), PropertyId::Text);

    // 验证不存在的名称返回 Unknown
    EXPECT_EQ(getPropertyIdFromName("unknown"), PropertyId::Unknown);
}

/**
 * 测试用例 12：PropertyId 获取名称（正常逻辑测试）
 * 测试目标：验证 getPropertyName 能通过枚举值找到对应的属性名
 */
TEST(PropertySystemTest, PropertyIdNames) {
    // 验证通过枚举找到正确的名称
    EXPECT_STREQ(getPropertyName(PropertyId::Width), "width");
    EXPECT_STREQ(getPropertyName(PropertyId::Text), "text");
    EXPECT_STREQ(getPropertyName(PropertyId::Unknown), "unknown");
}

/**
 * 测试用例 13：布局相关属性判断（条件测试）
 * 测试目标：验证 isLayoutAffectingProperty 能正确判断哪些属性会影响布局
 */
TEST(PropertySystemTest, LayoutAffectingProperties) {
    // 这些属性应该影响布局
    EXPECT_TRUE(isLayoutAffectingProperty(PropertyId::Width));
    EXPECT_TRUE(isLayoutAffectingProperty(PropertyId::Height));
    EXPECT_TRUE(isLayoutAffectingProperty(PropertyId::MarginLeft));

    // 这些属性不应该影响布局
    EXPECT_FALSE(isLayoutAffectingProperty(PropertyId::Text));
    EXPECT_FALSE(isLayoutAffectingProperty(PropertyId::BackgroundColor));
}

} // namespace test
} // namespace aether
