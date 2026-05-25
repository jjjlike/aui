#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>

namespace jaether {

// 前向声明
struct JJSONValue;

/**
 * JSON对象类型
 */
using JJSONObject = std::map<std::string, JJSONValue>;

/**
 * JSON数组类型
 */
using JJSONArray = std::vector<JJSONValue>;

/**
 * JSON值类型
 */
struct JJSONValue {
    using ValueType = std::variant<
        int,
        float,
        bool,
        std::string,
        std::nullptr_t,
        JJSONArray,
        JJSONObject
    >;

    ValueType value;

    JJSONValue() : value(nullptr) {}

    template <typename T>
    JJSONValue(T v) : value(std::move(v)) {}
};

/**
 * JSON解析器类
 */
class JJSONParser {
public:
    /**
     * 解析JSON字符串
     * @param json JSON字符串
     * @return 解析后的JSON值
     */
    static JJSONValue parse(const std::string& json);

    /**
     * 将JSON值序列化为字符串
     * @param value JSON值
     * @return JSON字符串
     */
    static std::string stringify(const JJSONValue& value);

private:
    static JJSONValue parseInternal(const std::string& json, size_t& pos);
    static void skipWhitespace(const std::string& json, size_t& pos);
    static std::string parseString(const std::string& json, size_t& pos);
    static JJSONValue parseNumber(const std::string& json, size_t& pos);
    static JJSONValue parseBooleanOrNull(const std::string& json, size_t& pos);
    static JJSONArray parseArray(const std::string& json, size_t& pos);
    static JJSONObject parseObject(const std::string& json, size_t& pos);
};

/**
 * JSON补丁类
 */
class JJSONPatch {
public:
    JJSONPatch();
    ~JJSONPatch();

    bool parse(const std::string& json);
    void apply(JJSONObject& root) const;
    std::string toString() const;
};

}
