/**
 * JSON 解析器模块 - 单元测试文件
 *
 * 功能说明：
 * - 测试 JSONParser 类的解析功能
 * - 测试 JSONValue 类的存储和访问
 * - 测试 JSON 的序列化功能
 * - 测试用例覆盖：所有 JSON 数据类型、嵌套结构
 */

#include "aether/JSONParser.h"
#include <gtest/gtest.h>
#include <string>

namespace aether {
namespace test {

/**
 * JSON 解析器测试的夹具类
 */
class JSONParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 暂时不需要特殊初始化
    }
};

/**
 * 测试用例 1：解析简单对象（正常逻辑测试）
 * 测试目标：验证能正确解析一个包含字符串和数字的 JSON 对象
 */
TEST_F(JSONParserTest, ParseSimpleObject) {
    // 准备 JSON 字符串：包含名称 "name" 和 "version"
    std::string json = R"({ "name": "Aether", "version": 1 })";
    // 解析 JSON
    auto node = JSONParser::parse(json);

    // 验证解析成功
    ASSERT_TRUE(std::holds_alternative<JSONObject>(node.value));

    // 获取对象
    const auto& obj = std::get<JSONObject>(node.value);

    // 验证 "name" 属性
    const auto& nameVal = obj.at("name");
    ASSERT_TRUE(std::holds_alternative<std::string>(nameVal.value));
    EXPECT_EQ(std::get<std::string>(nameVal.value), "Aether");

    // 验证 "version" 属性
    const auto& versionVal = obj.at("version");
    ASSERT_TRUE(std::holds_alternative<int>(versionVal.value));
    EXPECT_EQ(std::get<int>(versionVal.value), 1);
}

/**
 * 测试用例 2：解析数组（正常逻辑测试）
 * 测试目标：验证能正确解析包含多个元素的 JSON 数组
 */
TEST_F(JSONParserTest, ParseArray) {
    // 准备 JSON 数组：包含 1-5 的整数
    std::string json = R"([1, 2, 3, 4, 5])";
    // 解析 JSON
    auto node = JSONParser::parse(json);

    // 验证解析成功
    ASSERT_TRUE(std::holds_alternative<JSONArray>(node.value));

    // 获取数组
    const auto& arr = std::get<JSONArray>(node.value);

    // 验证数组大小是 5
    EXPECT_EQ(arr.size(), 5);

    // 遍历验证每个元素
    for (size_t i = 0; i < 5; ++i) {
        ASSERT_TRUE(std::holds_alternative<int>(arr[i].value));
        EXPECT_EQ(std::get<int>(arr[i].value), static_cast<int>(i + 1));
    }
}

/**
 * 测试用例 3：解析布尔值（正常逻辑测试）
 * 测试目标：验证能正确解析 true 和 false 值
 */
TEST_F(JSONParserTest, ParseBoolean) {
    // 准备 JSON 字符串：包含 true 和 false
    std::string json = R"({ "enabled": true, "disabled": false })";
    auto node = JSONParser::parse(json);

    ASSERT_TRUE(std::holds_alternative<JSONObject>(node.value));
    const auto& obj = std::get<JSONObject>(node.value);

    // 验证 true 值
    const auto& enabledVal = obj.at("enabled");
    ASSERT_TRUE(std::holds_alternative<bool>(enabledVal.value));
    EXPECT_TRUE(std::get<bool>(enabledVal.value));

    // 验证 false 值
    const auto& disabledVal = obj.at("disabled");
    ASSERT_TRUE(std::holds_alternative<bool>(disabledVal.value));
    EXPECT_FALSE(std::get<bool>(disabledVal.value));
}

/**
 * 测试用例 4：解析浮点数（正常逻辑测试）
 * 测试目标：验证能正确解析浮点数值
 */
TEST_F(JSONParserTest, ParseFloat) {
    // 准备 JSON 字符串：包含浮点数
    std::string json = R"({ "pi": 3.14159, "half": 0.5 })";
    auto node = JSONParser::parse(json);

    ASSERT_TRUE(std::holds_alternative<JSONObject>(node.value));
    const auto& obj = std::get<JSONObject>(node.value);

    // 验证 pi 值
    const auto& piVal = obj.at("pi");
    ASSERT_TRUE(std::holds_alternative<float>(piVal.value));
    EXPECT_FLOAT_EQ(std::get<float>(piVal.value), 3.14159f);
}

/**
 * 测试用例 5：解析 null 值（正常逻辑测试）
 * 测试目标：验证能正确解析 null 值
 */
TEST_F(JSONParserTest, ParseNull) {
    // 准备 JSON 字符串：包含 null
    std::string json = R"({ "value": null })";
    auto node = JSONParser::parse(json);

    ASSERT_TRUE(std::holds_alternative<JSONObject>(node.value));
    const auto& obj = std::get<JSONObject>(node.value);

    // 验证 null 值
    const auto& valueVal = obj.at("value");
    ASSERT_TRUE(std::holds_alternative<std::nullptr_t>(valueVal.value));
}

/**
 * 测试用例 6：序列化对象（正常逻辑测试）
 * 测试目标：验证能正确将 JSONValue 对象序列化为字符串
 */
TEST_F(JSONParserTest, StringifyObject) {
    // 创建一个 JSON 对象，包含名称和数值
    JSONObject obj;
    obj["name"] = JSONValue(std::string("Test"));
    obj["value"] = JSONValue(42);

    // 序列化为字符串
    std::string json = JSONParser::stringify(JSONValue(std::move(obj)));

    // 验证生成的字符串不为空
    EXPECT_FALSE(json.empty());
    // 验证包含我们需要的字段都在
    EXPECT_NE(json.find("name"), std::string::npos);
    EXPECT_NE(json.find("Test"), std::string::npos);
}

/**
 * 测试用例 7：嵌套对象（分支测试）
 * 测试目标：验证能正确解析多层嵌套的 JSON 对象
 */
TEST_F(JSONParserTest, NestedObjects) {
    // 准备嵌套结构的 JSON 字符串
    std::string json = R"({ "outer": { "inner": { "value": 100 } } })";
    auto node = JSONParser::parse(json);

    ASSERT_TRUE(std::holds_alternative<JSONObject>(node.value));
    const auto& obj = std::get<JSONObject>(node.value);

    // 访问外层对象
    const auto& outerVal = obj.at("outer");
    ASSERT_TRUE(std::holds_alternative<JSONObject>(outerVal.value));
    const auto& outer = std::get<JSONObject>(outerVal.value);

    // 访问内层对象
    const auto& innerVal = outer.at("inner");
    ASSERT_TRUE(std::holds_alternative<JSONObject>(innerVal.value));
    const auto& inner = std::get<JSONObject>(innerVal.value);

    // 验证最终值
    const auto& valueVal = inner.at("value");
    ASSERT_TRUE(std::holds_alternative<int>(valueVal.value));
    EXPECT_EQ(std::get<int>(valueVal.value), 100);
}

/**
 * 测试用例 8：包含对象的数组（分支测试）
 * 测试目标：验证能正确解析包含对象元素的数组
 */
TEST_F(JSONParserTest, ArrayWithObjects) {
    // 准备包含对象的数组
    std::string json = R"([{ "id": 1, "name": "Item 1" }, { "id": 2, "name": "Item 2" }])";
    auto node = JSONParser::parse(json);

    ASSERT_TRUE(std::holds_alternative<JSONArray>(node.value));
    const auto& arr = std::get<JSONArray>(node.value);

    // 验证数组大小
    EXPECT_EQ(arr.size(), 2);

    // 验证第一个对象的结构
    ASSERT_TRUE(std::holds_alternative<JSONObject>(arr[0].value));
    const auto& obj0 = std::get<JSONObject>(arr[0].value);
    ASSERT_TRUE(std::holds_alternative<int>(obj0.at("id").value));
    EXPECT_EQ(std::get<int>(obj0.at("id").value), 1);
}

/**
 * 测试用例 9：属性存在检查（正常逻辑测试）
 * 测试目标：验证能正确判断对象是否有某个属性
 */
TEST_F(JSONParserTest, HasProperty) {
    // 准备包含属性的对象
    std::string json = R"({ "key1": "value1", "key2": "value2" })";
    auto node = JSONParser::parse(json);

    ASSERT_TRUE(std::holds_alternative<JSONObject>(node.value));
    const auto& obj = std::get<JSONObject>(node.value);

    // 验证存在的属性
    EXPECT_NE(obj.find("key1"), obj.end());
    EXPECT_NE(obj.find("key2"), obj.end());
    // 验证不存在的属性
    EXPECT_EQ(obj.find("key3"), obj.end());
}

} // namespace test
} // namespace aether
