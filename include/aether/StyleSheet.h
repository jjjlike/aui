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

#include "types.h"
#include <string>

namespace jaether {

/**
 * 统一默认样式表结构体
 * 
 * 集中管理所有控件的视觉参数（颜色、字号、圆角、间距等）
 * 提供浅色主题和深色主题的工厂方法
 * 用户可直接修改字段值以定制主题，也可通过JRendererFacade::setStyleSheet整体替换
 */
struct JStyleSheet {
    // ===== 全局默认 =====
    std::string defaultFontName = "Microsoft YaHei";       // 默认中文字体
    JColor clearColor = JColor(0.96f, 0.96f, 0.96f, 1.0f); // 画布底色（浅灰白）

    // ===== Container 容器控件 =====
    JColor containerBackground = JColor(0.95f, 0.95f, 0.95f, 1.0f); // 柔和灰色背景

    // ===== Button 按钮控件 =====
    JColor buttonBackground        = JColor(0.2f, 0.5f, 0.9f, 1.0f);    // 默认蓝色背景
    JColor buttonPrimaryBackground  = JColor(0.15f, 0.45f, 0.85f, 1.0f);  // primary 深蓝
    JColor buttonHoverBackground    = JColor(1.0f, 0.85f, 0.0f, 1.0f);    // 悬停黄色
    JColor buttonTextColor          = JColor(1.0f, 1.0f, 1.0f, 1.0f);     // 按钮文字白色
    float  buttonFontSize           = 14.0f;                               // 按钮文字大小
    float  buttonRadius             = 5.0f;                                // 按钮圆角半径

    // ===== Text 文本控件 =====
    JColor textColor     = JColor(0.1f, 0.1f, 0.1f, 1.0f); // 深灰黑色（接近纯黑）
    float  textFontSize  = 14.0f;                           // 文本默认字号

    // ===== Input 输入框控件 =====
    JColor inputBackground   = JColor(1.0f, 1.0f, 1.0f, 1.0f);     // 白色背景
    JColor inputBorderColor  = JColor(0.6f, 0.6f, 0.6f, 1.0f);     // 灰色边框
    JColor inputTextColor    = JColor(0.1f, 0.1f, 0.1f, 1.0f);     // 深灰黑文字
    float  inputFontSize     = 14.0f;                               // 输入框文字大小
    float  inputBorderWidth  = 1.0f;                                // 边框线宽
    float  inputTextPadding  = 6.0f;                                // 输入文字左边距

    // ===== Card 卡片控件 =====
    JColor cardBackground   = JColor(1.0f, 1.0f, 1.0f, 1.0f);       // 白色背景
    JColor cardBorderColor  = JColor(0.85f, 0.85f, 0.85f, 1.0f);    // 浅灰边框
    JColor cardShadowColor  = JColor(0.0f, 0.0f, 0.0f, 0.08f);      // 半透明阴影
    float  cardRadius       = 8.0f;                                  // 卡片圆角半径
    float  cardBorderWidth  = 1.0f;                                  // 边框线宽
    float  cardShadowOffset = 3.0f;                                  // 阴影偏移量

    // ===== CheckBox 复选框控件（预留） =====
    float  checkboxSize        = 18.0f;
    JColor checkboxCheckedBg   = JColor(0.2f, 0.5f, 0.9f, 1.0f);
    JColor checkboxUncheckedBg = JColor(1.0f, 1.0f, 1.0f, 1.0f);
    JColor checkboxBorderColor = JColor(0.5f, 0.5f, 0.5f, 1.0f);

    // ===== ScrollView 滚动视图控件（预留） =====
    JColor scrollbarColor = JColor(0.7f, 0.7f, 0.7f, 0.5f);
    float  scrollbarWidth = 8.0f;

    // ===== Slider 滑块控件（预留） =====
    JColor sliderTrackColor  = JColor(0.85f, 0.85f, 0.85f, 1.0f);
    JColor sliderThumbColor  = JColor(0.2f, 0.5f, 0.9f, 1.0f);
    float  sliderTrackHeight = 4.0f;
    float  sliderThumbRadius = 10.0f;

    /**
     * 创建默认浅色主题样式表
     * @return 浅色主题的JStyleSheet实例
     */
    static JStyleSheet light();

    /**
     * 创建深色主题样式表
     * @return 深色主题的JStyleSheet实例
     */
    static JStyleSheet dark();
};

} // namespace jaether
