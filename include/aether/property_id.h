#pragma once

#include <cstdint>
#include <string_view>

namespace aether {

/**
 * 属性ID枚举
 * 
 * 定义了所有可访问的组件属性类型
 * 使用16位整数确保高效存储
 */
enum class PropertyId : uint16_t {
    Unknown = 0,
    
    // 基础属性
    Text = 1,           // 文本内容
    Enabled = 2,        // 是否启用
    Visible = 3,      // 是否可见
    
    // 布局属性 - 位置和尺寸
    X = 8,             // X坐标
    Y = 9,             // Y坐标
    Width = 10,          // 宽度
    Height = 11,        // 高度
    MinWidth = 12,      // 最小宽度
    MinHeight = 13,     // 最小高度
    MaxWidth = 14,      // 最大宽度
    MaxHeight = 15,     // 最大高度
    
    // 外边距
    MarginLeft = 20,    // 左外边距
    MarginTop = 21,     // 上外边距
    MarginRight = 22,   // 右外边距
    MarginBottom = 23,  // 下外边距
    
    // 内边距
    PaddingLeft = 30,    // 左内边距
    PaddingTop = 31,     // 上内边距
    PaddingRight = 32,   // 右内边距
    PaddingBottom = 33, // 下内边距
    
    // Flex布局属性
    FlexGrow = 40,      // Flex放大因子
    FlexShrink = 41,    // Flex收缩因子
    FlexBasis = 42,     // Flex基准尺寸
    FlexDirection = 43, // Flex主轴方向
    FlexWrap = 44,       // Flex换行方式
    
    // Flex对齐方式
    JustifyContent = 50, // 主轴对齐方式
    AlignItems = 51,     // 交叉轴对齐方式（单行）
    AlignContent = 52,     // 交叉轴对齐方式（多行）
    AlignSelf = 53,       // 自身对齐方式
    
    // 定位属性
    PositionType = 60,   // 定位类型
    PositionLeft = 61,   // 定位左偏移
    PositionTop = 62,    // 定位上偏移
    PositionRight = 63,  // 定位右偏移
    PositionBottom = 64, // 定位下偏移
    
    // 样式属性 - 外观
    BackgroundColor = 70, // 背景色
    BorderColor = 71,    // 边框颜色
    BorderWidth = 72,    // 边框宽度
    BorderRadius = 73,   // 圆角半径
    
    // 文本样式
    FontSize = 80,       // 字体大小
    FontFamily = 81,     // 字体族
    FontWeight = 82,      // 字体粗细
    TextColor = 83,      // 文本颜色
    TextAlign = 84,      // 文本对齐方式
    
    // 层级属性
    ZIndex = 90,         // Z轴层级
    
    // 用户自定义属性起始值
    UserDefined = 1000
};

/**
 * 最大属性ID常量
 * 用于数组大小等场景
 */
constexpr size_t MAX_PROPERTY_ID = 2000;

/**
 * PropertyId哈希函数结构体
 * 
 * 用于将PropertyId转换为size_t，以便在哈希表中使用
 */
struct PropertyIdHash {
    /**
     * 计算PropertyId的哈希值
     * @param id 属性ID
     * @return 哈希值
     */
    size_t operator()(PropertyId id) const {
        return static_cast<size_t>(id);
    }
};

/**
 * 根据属性名称获取属性ID
 * 
 * @param name 属性名称字符串
 * @return 对应的PropertyId，如果未找到返回Unknown
 */
constexpr PropertyId getPropertyIdFromName(std::string_view name) {
    if (name == "text") return PropertyId::Text;
    if (name == "enabled") return PropertyId::Enabled;
    if (name == "visible") return PropertyId::Visible;
    
    if (name == "x") return PropertyId::X;
    if (name == "y") return PropertyId::Y;
    if (name == "width") return PropertyId::Width;
    if (name == "height") return PropertyId::Height;
    if (name == "minWidth") return PropertyId::MinWidth;
    if (name == "minHeight") return PropertyId::MinHeight;
    if (name == "maxWidth") return PropertyId::MaxWidth;
    if (name == "maxHeight") return PropertyId::MaxHeight;
    
    if (name == "marginLeft") return PropertyId::MarginLeft;
    if (name == "marginTop") return PropertyId::MarginTop;
    if (name == "marginRight") return PropertyId::MarginRight;
    if (name == "marginBottom") return PropertyId::MarginBottom;
    
    if (name == "paddingLeft") return PropertyId::PaddingLeft;
    if (name == "paddingTop") return PropertyId::PaddingTop;
    if (name == "paddingRight") return PropertyId::PaddingRight;
    if (name == "paddingBottom") return PropertyId::PaddingBottom;
    
    if (name == "flexGrow") return PropertyId::FlexGrow;
    if (name == "flexShrink") return PropertyId::FlexShrink;
    if (name == "flexBasis") return PropertyId::FlexBasis;
    if (name == "flexDirection") return PropertyId::FlexDirection;
    if (name == "flexWrap") return PropertyId::FlexWrap;
    
    if (name == "justifyContent") return PropertyId::JustifyContent;
    if (name == "alignItems") return PropertyId::AlignItems;
    if (name == "alignContent") return PropertyId::AlignContent;
    if (name == "alignSelf") return PropertyId::AlignSelf;
    
    if (name == "positionType") return PropertyId::PositionType;
    if (name == "positionLeft") return PropertyId::PositionLeft;
    if (name == "positionTop") return PropertyId::PositionTop;
    if (name == "positionRight") return PropertyId::PositionRight;
    if (name == "positionBottom") return PropertyId::PositionBottom;
    
    if (name == "backgroundColor") return PropertyId::BackgroundColor;
    if (name == "borderColor") return PropertyId::BorderColor;
    if (name == "borderWidth") return PropertyId::BorderWidth;
    if (name == "borderRadius") return PropertyId::BorderRadius;
    
    if (name == "fontSize") return PropertyId::FontSize;
    if (name == "fontFamily") return PropertyId::FontFamily;
    if (name == "fontWeight") return PropertyId::FontWeight;
    if (name == "textColor") return PropertyId::TextColor;
    if (name == "textAlign") return PropertyId::TextAlign;
    
    if (name == "zIndex") return PropertyId::ZIndex;
    
    return PropertyId::Unknown;
}

/**
 * 根据属性ID获取属性名称
 * 
 * @param id 属性ID
 * @return 对应的属性名称字符串
 */
constexpr const char* getPropertyName(PropertyId id) {
    switch (id) {
        case PropertyId::Text: return "text";
        case PropertyId::Enabled: return "enabled";
        case PropertyId::Visible: return "visible";
        case PropertyId::X: return "x";
        case PropertyId::Y: return "y";
        case PropertyId::Width: return "width";
        case PropertyId::Height: return "height";
        case PropertyId::MinWidth: return "minWidth";
        case PropertyId::MinHeight: return "minHeight";
        case PropertyId::MaxWidth: return "maxWidth";
        case PropertyId::MaxHeight: return "maxHeight";
        case PropertyId::MarginLeft: return "marginLeft";
        case PropertyId::MarginTop: return "marginTop";
        case PropertyId::MarginRight: return "marginRight";
        case PropertyId::MarginBottom: return "marginBottom";
        case PropertyId::PaddingLeft: return "paddingLeft";
        case PropertyId::PaddingTop: return "paddingTop";
        case PropertyId::PaddingRight: return "paddingRight";
        case PropertyId::PaddingBottom: return "paddingBottom";
        case PropertyId::FlexGrow: return "flexGrow";
        case PropertyId::FlexShrink: return "flexShrink";
        case PropertyId::FlexBasis: return "flexBasis";
        case PropertyId::FlexDirection: return "flexDirection";
        case PropertyId::FlexWrap: return "flexWrap";
        case PropertyId::JustifyContent: return "justifyContent";
        case PropertyId::AlignItems: return "alignItems";
        case PropertyId::AlignContent: return "alignContent";
        case PropertyId::AlignSelf: return "alignSelf";
        case PropertyId::PositionType: return "positionType";
        case PropertyId::PositionLeft: return "positionLeft";
        case PropertyId::PositionTop: return "positionTop";
        case PropertyId::PositionRight: return "positionRight";
        case PropertyId::PositionBottom: return "positionBottom";
        case PropertyId::BackgroundColor: return "backgroundColor";
        case PropertyId::BorderColor: return "borderColor";
        case PropertyId::BorderWidth: return "borderWidth";
        case PropertyId::BorderRadius: return "borderRadius";
        case PropertyId::FontSize: return "fontSize";
        case PropertyId::FontFamily: return "fontFamily";
        case PropertyId::FontWeight: return "fontWeight";
        case PropertyId::TextColor: return "textColor";
        case PropertyId::TextAlign: return "textAlign";
        case PropertyId::ZIndex: return "zIndex";
        default: return "unknown";
    }
}

/**
 * 判断属性是否会影响布局
 * 
 * @param id 属性ID
 * @return 如果属性改变时是否需要重新布局则返回true
 */
constexpr bool isLayoutAffectingProperty(PropertyId id) {
    switch (id) {
        case PropertyId::X:
        case PropertyId::Y:
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
        case PropertyId::FlexDirection:
        case PropertyId::FlexWrap:
        case PropertyId::JustifyContent:
        case PropertyId::AlignItems:
        case PropertyId::AlignContent:
        case PropertyId::AlignSelf:
        case PropertyId::PositionType:
        case PropertyId::PositionLeft:
        case PropertyId::PositionTop:
        case PropertyId::PositionRight:
        case PropertyId::PositionBottom:
            return true;
        default:
            return false;
    }
}

}
