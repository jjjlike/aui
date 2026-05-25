// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


#include "aether/JSONParser.h"
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace jaether {

JJSONValue JJSONParser::parse(const std::string& json) {
    size_t pos = 0;
    return parseInternal(json, pos);
}

JJSONValue JJSONParser::parseInternal(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);

    if (pos >= json.size()) {
        return JJSONValue();
    }

    char c = json[pos];
    if (c == '{') {
        return JJSONValue(parseObject(json, pos));
    } else if (c == '[') {
        return JJSONValue(parseArray(json, pos));
    } else if (c == '"') {
        return JJSONValue(parseString(json, pos));
    } else if (c == 't' || c == 'f' || c == 'n') {
        return parseBooleanOrNull(json, pos);
    } else {
        return parseNumber(json, pos);
    }
}

std::string JJSONParser::stringify(const JJSONValue& value) {
    std::ostringstream oss;

    if (std::holds_alternative<int>(value.value)) {
        oss << std::get<int>(value.value);
    } else if (std::holds_alternative<float>(value.value)) {
        oss << std::get<float>(value.value);
    } else if (std::holds_alternative<bool>(value.value)) {
        oss << (std::get<bool>(value.value) ? "true" : "false");
    } else if (std::holds_alternative<std::string>(value.value)) {
        const auto& str = std::get<std::string>(value.value);
        oss << '"';
        for (char c : str) {
            if (c == '"' || c == '\\') {
                oss << '\\' << c;
            } else if (c == '\n') {
                oss << "\\n";
            } else if (c == '\r') {
                oss << "\\r";
            } else if (c == '\t') {
                oss << "\\t";
            } else {
                oss << c;
            }
        }
        oss << '"';
    } else if (std::holds_alternative<std::nullptr_t>(value.value)) {
        oss << "null";
    } else if (std::holds_alternative<JJSONArray>(value.value)) {
        const auto& arr = std::get<JJSONArray>(value.value);
        oss << '[';
        bool first = true;
        for (const auto& item : arr) {
            if (!first) {
                oss << ',';
            }
            oss << stringify(item);
            first = false;
        }
        oss << ']';
    } else if (std::holds_alternative<JJSONObject>(value.value)) {
        const auto& obj = std::get<JJSONObject>(value.value);
        oss << '{';
        bool first = true;
        for (const auto& kv : obj) {
            if (!first) {
                oss << ',';
            }
            oss << '"' << kv.first << '"' << ':' << stringify(kv.second);
            first = false;
        }
        oss << '}';
    }

    return oss.str();
}

void JJSONParser::skipWhitespace(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
}

std::string JJSONParser::parseString(const std::string& json, size_t& pos) {
    ++pos; // skip opening "
    std::string result;

    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;
            char esc = json[pos];
            switch (esc) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += esc; break;
            }
        } else {
            result += json[pos];
        }
        ++pos;
    }

    ++pos; // skip closing "
    return result;
}

JJSONValue JJSONParser::parseNumber(const std::string& json, size_t& pos) {
    size_t start = pos;
    bool isFloat = false;

    if (json[pos] == '-') {
        ++pos;
    }

    while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }

    if (pos < json.size() && json[pos] == '.') {
        isFloat = true;
        ++pos;
        while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) {
            ++pos;
        }
    }

    if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
        isFloat = true;
        ++pos;
        if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) {
            ++pos;
        }
        while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) {
            ++pos;
        }
    }

    std::string numStr = json.substr(start, pos - start);

    if (isFloat) {
        return JJSONValue(std::stof(numStr));
    } else {
        return JJSONValue(std::stoi(numStr));
    }
}

JJSONValue JJSONParser::parseBooleanOrNull(const std::string& json, size_t& pos) {
    if (json.substr(pos, 4) == "true") {
        pos += 4;
        return JJSONValue(true);
    } else if (json.substr(pos, 5) == "false") {
        pos += 5;
        return JJSONValue(false);
    } else if (json.substr(pos, 4) == "null") {
        pos += 4;
        return JJSONValue();
    }
    throw std::runtime_error("Invalid JSON value");
}

JJSONArray JJSONParser::parseArray(const std::string& json, size_t& pos) {
    ++pos; // skip [
    JJSONArray arr;

    skipWhitespace(json, pos);
    while (pos < json.size() && json[pos] != ']') {
        arr.push_back(parseInternal(json, pos));
        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            skipWhitespace(json, pos);
        }
    }

    ++pos; // skip ]
    return arr;
}

JJSONObject JJSONParser::parseObject(const std::string& json, size_t& pos) {
    ++pos; // skip {
    JJSONObject obj;

    skipWhitespace(json, pos);
    while (pos < json.size() && json[pos] != '}') {
        auto key = parseString(json, pos);
        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ':') {
            ++pos;
            skipWhitespace(json, pos);
            obj[key] = parseInternal(json, pos);
        }
        skipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            skipWhitespace(json, pos);
        }
    }

    ++pos; // skip }
    return obj;
}

JJSONPatch::JJSONPatch() {}

JJSONPatch::~JJSONPatch() {}

bool JJSONPatch::parse(const std::string& json) {
    return false;
}

void JJSONPatch::apply(JJSONObject& root) const {}

std::string JJSONPatch::toString() const {
    return "[]";
}

}
