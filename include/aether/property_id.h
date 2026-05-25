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

#include <cstdint>
#include <string_view>

namespace jaether {

/**
 * 属性ID枚举
 * 
 * 定义了所有可访问的组件属性类型
 * 使用16位整数确保高效存储
 */
enum class JPropertyId : uint16_t {
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
    JFlexDirection = 43, // Flex主轴方向
    JFlexWrap = 44,       // Flex换行方式
    
    // Flex对齐方式
    JJustifyContent = 50, // 主轴对齐方式
    JAlignItems = 51,     // 交叉轴对齐方式（单行）
    JAlignContent = 52,     // 交叉轴对齐方式（多行）
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
    size_t operator()(JPropertyId id) const {
        return static_cast<size_t>(id);
    }
};

/**
 * 根据属性名称获取属性ID
 * 
 * @param name 属性名称字符串
 * @return 对应的PropertyId，如果未找到返回Unknown
 */
constexpr JPropertyId getPropertyIdFromName(std::string_view name) {
    if (name == "text") return JPropertyId::Text;
    if (name == "enabled") return JPropertyId::Enabled;
    if (name == "visible") return JPropertyId::Visible;
    
    if (name == "x") return JPropertyId::X;
    if (name == "y") return JPropertyId::Y;
    if (name == "width") return JPropertyId::Width;
    if (name == "height") return JPropertyId::Height;
    if (name == "minWidth") return JPropertyId::MinWidth;
    if (name == "minHeight") return JPropertyId::MinHeight;
    if (name == "maxWidth") return JPropertyId::MaxWidth;
    if (name == "maxHeight") return JPropertyId::MaxHeight;
    
    if (name == "marginLeft") return JPropertyId::MarginLeft;
    if (name == "marginTop") return JPropertyId::MarginTop;
    if (name == "marginRight") return JPropertyId::MarginRight;
    if (name == "marginBottom") return JPropertyId::MarginBottom;
    
    if (name == "paddingLeft") return JPropertyId::PaddingLeft;
    if (name == "paddingTop") return JPropertyId::PaddingTop;
    if (name == "paddingRight") return JPropertyId::PaddingRight;
    if (name == "paddingBottom") return JPropertyId::PaddingBottom;
    
    if (name == "flexGrow") return JPropertyId::FlexGrow;
    if (name == "flexShrink") return JPropertyId::FlexShrink;
    if (name == "flexBasis") return JPropertyId::FlexBasis;
    if (name == "flexDirection") return JPropertyId::JFlexDirection;
    if (name == "flexWrap") return JPropertyId::JFlexWrap;
    
    if (name == "justifyContent") return JPropertyId::JJustifyContent;
    if (name == "alignItems") return JPropertyId::JAlignItems;
    if (name == "alignContent") return JPropertyId::JAlignContent;
    if (name == "alignSelf") return JPropertyId::AlignSelf;
    
    if (name == "positionType") return JPropertyId::PositionType;
    if (name == "positionLeft") return JPropertyId::PositionLeft;
    if (name == "positionTop") return JPropertyId::PositionTop;
    if (name == "positionRight") return JPropertyId::PositionRight;
    if (name == "positionBottom") return JPropertyId::PositionBottom;
    
    if (name == "backgroundColor") return JPropertyId::BackgroundColor;
    if (name == "borderColor") return JPropertyId::BorderColor;
    if (name == "borderWidth") return JPropertyId::BorderWidth;
    if (name == "borderRadius") return JPropertyId::BorderRadius;
    
    if (name == "fontSize") return JPropertyId::FontSize;
    if (name == "fontFamily") return JPropertyId::FontFamily;
    if (name == "fontWeight") return JPropertyId::FontWeight;
    if (name == "textColor") return JPropertyId::TextColor;
    if (name == "textAlign") return JPropertyId::TextAlign;
    
    if (name == "zIndex") return JPropertyId::ZIndex;
    
    return JPropertyId::Unknown;
}

/**
 * 根据属性ID获取属性名称
 * 
 * @param id 属性ID
 * @return 对应的属性名称字符串
 */
constexpr const char* getPropertyName(JPropertyId id) {
    switch (id) {
        case JPropertyId::Text: return "text";
        case JPropertyId::Enabled: return "enabled";
        case JPropertyId::Visible: return "visible";
        case JPropertyId::X: return "x";
        case JPropertyId::Y: return "y";
        case JPropertyId::Width: return "width";
        case JPropertyId::Height: return "height";
        case JPropertyId::MinWidth: return "minWidth";
        case JPropertyId::MinHeight: return "minHeight";
        case JPropertyId::MaxWidth: return "maxWidth";
        case JPropertyId::MaxHeight: return "maxHeight";
        case JPropertyId::MarginLeft: return "marginLeft";
        case JPropertyId::MarginTop: return "marginTop";
        case JPropertyId::MarginRight: return "marginRight";
        case JPropertyId::MarginBottom: return "marginBottom";
        case JPropertyId::PaddingLeft: return "paddingLeft";
        case JPropertyId::PaddingTop: return "paddingTop";
        case JPropertyId::PaddingRight: return "paddingRight";
        case JPropertyId::PaddingBottom: return "paddingBottom";
        case JPropertyId::FlexGrow: return "flexGrow";
        case JPropertyId::FlexShrink: return "flexShrink";
        case JPropertyId::FlexBasis: return "flexBasis";
        case JPropertyId::JFlexDirection: return "flexDirection";
        case JPropertyId::JFlexWrap: return "flexWrap";
        case JPropertyId::JJustifyContent: return "justifyContent";
        case JPropertyId::JAlignItems: return "alignItems";
        case JPropertyId::JAlignContent: return "alignContent";
        case JPropertyId::AlignSelf: return "alignSelf";
        case JPropertyId::PositionType: return "positionType";
        case JPropertyId::PositionLeft: return "positionLeft";
        case JPropertyId::PositionTop: return "positionTop";
        case JPropertyId::PositionRight: return "positionRight";
        case JPropertyId::PositionBottom: return "positionBottom";
        case JPropertyId::BackgroundColor: return "backgroundColor";
        case JPropertyId::BorderColor: return "borderColor";
        case JPropertyId::BorderWidth: return "borderWidth";
        case JPropertyId::BorderRadius: return "borderRadius";
        case JPropertyId::FontSize: return "fontSize";
        case JPropertyId::FontFamily: return "fontFamily";
        case JPropertyId::FontWeight: return "fontWeight";
        case JPropertyId::TextColor: return "textColor";
        case JPropertyId::TextAlign: return "textAlign";
        case JPropertyId::ZIndex: return "zIndex";
        default: return "unknown";
    }
}

/**
 * 判断属性是否会影响布局
 * 
 * @param id 属性ID
 * @return 如果属性改变时是否需要重新布局则返回true
 */
constexpr bool isLayoutAffectingProperty(JPropertyId id) {
    switch (id) {
        case JPropertyId::X:
        case JPropertyId::Y:
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
        case JPropertyId::JFlexDirection:
        case JPropertyId::JFlexWrap:
        case JPropertyId::JJustifyContent:
        case JPropertyId::JAlignItems:
        case JPropertyId::JAlignContent:
        case JPropertyId::AlignSelf:
        case JPropertyId::PositionType:
        case JPropertyId::PositionLeft:
        case JPropertyId::PositionTop:
        case JPropertyId::PositionRight:
        case JPropertyId::PositionBottom:
            return true;
        default:
            return false;
    }
}

}
