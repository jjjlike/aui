#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

namespace aether {

// 前向声明
struct JSONValue;

/**
 * JSON对象类型
 */
using JSONObject = std::map<std::string, JSONValue>;

/**
 * JSON数组类型
 */
using JSONArray = std::vector<JSONValue>;

/**
 * JSON值类型
 */
struct JSONValue {
    using ValueType = std::variant<
        int,
        float,
        bool,
        std::string,
        std::nullptr_t,
        JSONArray,
        JSONObject
    >;

    ValueType value;

    JSONValue() : value(nullptr) {}

    template <typename T>
    JSONValue(T v) : value(std::move(v)) {}
};

/**
 * JSON解析器类
 */
class JSONParser {
public:
    /**
     * 解析JSON字符串
     * @param json JSON字符串
     * @return 解析后的JSON值
     */
    static JSONValue parse(const std::string& json);

    /**
     * 将JSON值序列化为字符串
     * @param value JSON值
     * @return JSON字符串
     */
    static std::string stringify(const JSONValue& value);

private:
    static void skipWhitespace(const std::string& json, size_t& pos);
    static std::string parseString(const std::string& json, size_t& pos);
    static JSONValue parseNumber(const std::string& json, size_t& pos);
    static JSONValue parseBooleanOrNull(const std::string& json, size_t& pos);
    static JSONArray parseArray(const std::string& json, size_t& pos);
    static JSONObject parseObject(const std::string& json, size_t& pos);
};

/**
 * JSON补丁类
 */
class JSONPatch {
public:
    JSONPatch();
    ~JSONPatch();

    bool parse(const std::string& json);
    void apply(JSONObject& root) const;
    std::string toString() const;
};

}
