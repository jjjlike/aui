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

namespace aether {

// 将属性值转换为字符串表示
// 返回值: 属性值的字符串形式
std::string PropertyValue::toString() const {
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
        } else if constexpr (std::is_same_v<T, Color>) {
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
        } else if constexpr (std::is_same_v<T, Rect>) {
            // 矩形格式化为 {x,y,w,h}
            std::ostringstream oss;
            oss << "{" << arg.x << "," << arg.y << "," << arg.width << "," << arg.height << "}";
            return oss.str();
        } else if constexpr (std::is_same_v<T, FlexDirection>) {
            // Flex方向枚举转换
            switch (arg) {
                case FlexDirection::Row: return "row";
                case FlexDirection::Column: return "column";
            }
        } else if constexpr (std::is_same_v<T, FlexWrap>) {
            // Flex换行枚举转换
            switch (arg) {
                case FlexWrap::NoWrap: return "nowrap";
                case FlexWrap::Wrap: return "wrap";
            }
        } else if constexpr (std::is_same_v<T, JustifyContent>) {
            // 主轴对齐枚举转换
            switch (arg) {
                case JustifyContent::FlexStart: return "flex-start";
                case JustifyContent::Center: return "center";
                case JustifyContent::FlexEnd: return "flex-end";
                case JustifyContent::SpaceBetween: return "space-between";
                case JustifyContent::SpaceAround: return "space-around";
            }
        } else if constexpr (std::is_same_v<T, AlignItems>) {
            // 交叉轴对齐枚举转换
            switch (arg) {
                case AlignItems::FlexStart: return "flex-start";
                case AlignItems::Center: return "center";
                case AlignItems::FlexEnd: return "flex-end";
                case AlignItems::Stretch: return "stretch";
            }
        } else if constexpr (std::is_same_v<T, AlignContent>) {
            // 多行内容对齐枚举转换
            switch (arg) {
                case AlignContent::FlexStart: return "flex-start";
                case AlignContent::Center: return "center";
                case AlignContent::FlexEnd: return "flex-end";
                case AlignContent::SpaceBetween: return "space-between";
                case AlignContent::SpaceAround: return "space-around";
                case AlignContent::Stretch: return "stretch";
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
PropertyValue PropertyValue::fromString(PropertyId id, const std::string& str) {
    switch (id) {
        // 字符串类型属性
        case PropertyId::Text:
        case PropertyId::FontFamily:
            return PropertyValue{str};
            
        // 布尔类型属性
        case PropertyId::Enabled:
        case PropertyId::Visible:
            return PropertyValue{str == "true" || str == "1"};
            
        // 数值类型属性（支持整数和浮点数）
        case PropertyId::Width:
        case PropertyId::Height:
        case PropertyId::MinWidth:
        case PropertyId::MinHeight:
        case PropertyId::MaxWidth:
        case PropertyId::MaxHeight:
        case PropertyId::MarginLeft:
        case PropertyId::MarginTop:
        case PropertyId::MarginRight:
        case PropertyId::MarginBottom:
        case PropertyId::PaddingLeft:
        case PropertyId::PaddingTop:
        case PropertyId::PaddingRight:
        case PropertyId::PaddingBottom:
        case PropertyId::FlexGrow:
        case PropertyId::FlexShrink:
        case PropertyId::FlexBasis:
        case PropertyId::FontSize:
        case PropertyId::FontWeight:
        case PropertyId::BorderWidth:
        case PropertyId::BorderRadius:
        case PropertyId::ZIndex:
        case PropertyId::PositionLeft:
        case PropertyId::PositionTop:
        case PropertyId::PositionRight:
        case PropertyId::PositionBottom:
            // 根据是否包含小数点决定解析为浮点还是整数
            if (str.find('.') != std::string::npos) {
                return PropertyValue{std::stof(str)};
            }
            return PropertyValue{std::stoi(str)};
            
        // 颜色类型属性
        case PropertyId::BackgroundColor:
        case PropertyId::BorderColor:
        case PropertyId::TextColor:
            if (str[0] == '#') {
                // 解析十六进制颜色
                uint32_t color = std::stoul(str.substr(1), nullptr, 16);
                return PropertyValue{Color::fromARGB(color)};
            }
            // 默认返回白色
            return PropertyValue{Color{static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)}};
            
        // Flex方向属性
        case PropertyId::FlexDirection:
            return PropertyValue{str == "column" ? FlexDirection::Column : FlexDirection::Row};
            
        // Flex换行属性
        case PropertyId::FlexWrap:
            return PropertyValue{str == "wrap" ? FlexWrap::Wrap : FlexWrap::NoWrap};
            
        default:
            return PropertyValue{};  // 未知属性返回空值
    }
}

} // namespace aether
