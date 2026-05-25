// PropertySystem.cpp
// 属性系统模块 - 管理UI组件的属性值和类型转换
//
// 功能:
// - 使用variant存储多种类型的属性值
// - 支持属性值到字符串的转换
// - 支持从字符串解析属性值
// - 处理各种UI属性类型（颜色、布局属性等）

#include "aether/ComponentStorage.h"
#include <sstream>
#include <iomanip>

namespace jaether {

// 将属性值转换为字符串表示
// 返回值: 属性值的字符串形式
std::string JPropertyValue::toString() const {
    // 使用visit模式根据实际类型进行转换
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        
        // 处理不同类型的值
        if constexpr (std::is_same_v<T, std::monostate>) {
            return "null";  // 空值
        } else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);  // 整数
        } else if constexpr (std::is_same_v<T, float>) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << arg;  // 浮点数保留2位小数
            return oss.str();
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";  // 布尔值
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg;  // 字符串直接返回
        } else if constexpr (std::is_same_v<T, JColor>) {
            // 颜色转换为十六进制格式
            std::ostringstream oss;
            oss << "#" << std::hex << std::setfill('0') 
                << std::setw(2) << (int)arg.r
                << std::setw(2) << (int)arg.g
                << std::setw(2) << (int)arg.b;
            if (arg.a != 255) {
                oss << std::setw(2) << (int)arg.a;  // 添加alpha通道
            }
            return oss.str();
        } else if constexpr (std::is_same_v<T, JRect>) {
            // 矩形格式化为 {x,y,w,h}
            std::ostringstream oss;
            oss << "{" << arg.x << "," << arg.y << "," << arg.width << "," << arg.height << "}";
            return oss.str();
        } else if constexpr (std::is_same_v<T, JFlexDirection>) {
            // Flex方向枚举转换
            switch (arg) {
                case JFlexDirection::Row: return "row";
                case JFlexDirection::Column: return "column";
            }
        } else if constexpr (std::is_same_v<T, JFlexWrap>) {
            // Flex换行枚举转换
            switch (arg) {
                case JFlexWrap::NoWrap: return "nowrap";
                case JFlexWrap::Wrap: return "wrap";
            }
        } else if constexpr (std::is_same_v<T, JJustifyContent>) {
            // 主轴对齐枚举转换
            switch (arg) {
                case JJustifyContent::FlexStart: return "flex-start";
                case JJustifyContent::Center: return "center";
                case JJustifyContent::FlexEnd: return "flex-end";
                case JJustifyContent::SpaceBetween: return "space-between";
                case JJustifyContent::SpaceAround: return "space-around";
            }
        } else if constexpr (std::is_same_v<T, JAlignItems>) {
            // 交叉轴对齐枚举转换
            switch (arg) {
                case JAlignItems::FlexStart: return "flex-start";
                case JAlignItems::Center: return "center";
                case JAlignItems::FlexEnd: return "flex-end";
                case JAlignItems::Stretch: return "stretch";
            }
        } else if constexpr (std::is_same_v<T, JAlignContent>) {
            // 多行内容对齐枚举转换
            switch (arg) {
                case JAlignContent::FlexStart: return "flex-start";
                case JAlignContent::Center: return "center";
                case JAlignContent::FlexEnd: return "flex-end";
                case JAlignContent::SpaceBetween: return "space-between";
                case JAlignContent::SpaceAround: return "space-around";
                case JAlignContent::Stretch: return "stretch";
            }
        }
        return "unknown";  // 未知类型
    }, value);
}

// 从字符串解析属性值
// 参数:
//   id - 属性ID，用于确定解析方式
//   str - 要解析的字符串
// 返回值: 解析后的属性值
JPropertyValue JPropertyValue::fromString(JPropertyId id, const std::string& str) {
    switch (id) {
        // 字符串类型属性
        case JPropertyId::Text:
        case JPropertyId::FontFamily:
            return JPropertyValue{str};
            
        // 布尔类型属性
        case JPropertyId::Enabled:
        case JPropertyId::Visible:
            return JPropertyValue{str == "true" || str == "1"};
            
        // 数值类型属性（支持整数和浮点数）
        case JPropertyId::Width:
        case JPropertyId::Height:
        case JPropertyId::MinWidth:
        case JPropertyId::MinHeight:
        case JPropertyId::MaxWidth:
        case JPropertyId::MaxHeight:
        case JPropertyId::MarginLeft:
        case JPropertyId::MarginTop:
        case JPropertyId::MarginRight:
        case JPropertyId::MarginBottom:
        case JPropertyId::PaddingLeft:
        case JPropertyId::PaddingTop:
        case JPropertyId::PaddingRight:
        case JPropertyId::PaddingBottom:
        case JPropertyId::FlexGrow:
        case JPropertyId::FlexShrink:
        case JPropertyId::FlexBasis:
        case JPropertyId::FontSize:
        case JPropertyId::FontWeight:
        case JPropertyId::BorderWidth:
        case JPropertyId::BorderRadius:
        case JPropertyId::ZIndex:
        case JPropertyId::PositionLeft:
        case JPropertyId::PositionTop:
        case JPropertyId::PositionRight:
        case JPropertyId::PositionBottom:
            // 根据是否包含小数点决定解析为浮点还是整数
            if (str.find('.') != std::string::npos) {
                return JPropertyValue{std::stof(str)};
            }
            return JPropertyValue{std::stoi(str)};
            
        // 颜色类型属性
        case JPropertyId::BackgroundColor:
        case JPropertyId::BorderColor:
        case JPropertyId::TextColor:
            if (str[0] == '#') {
                // 解析十六进制颜色
                uint32_t color = std::stoul(str.substr(1), nullptr, 16);
                return JPropertyValue{JColor::fromARGB(color)};
            }
            // 默认返回白色
            return JPropertyValue{JColor{static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)}};
            
        // Flex方向属性
        case JPropertyId::JFlexDirection:
            return JPropertyValue{str == "column" ? JFlexDirection::Column : JFlexDirection::Row};
            
        // Flex换行属性
        case JPropertyId::JFlexWrap:
            return JPropertyValue{str == "wrap" ? JFlexWrap::Wrap : JFlexWrap::NoWrap};
            
        default:
            return JPropertyValue{};  // 未知属性返回空值
    }
}

} // namespace jaether
