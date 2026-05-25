// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// StyleSheet.cpp
// 统一样式表模块 - 提供浅色/深色主题的工厂方法

#include "aether/StyleSheet.h"

namespace jaether {

// 创建默认浅色主题（与JStyleSheet默认构造值相同）
JStyleSheet JStyleSheet::light() {
    return JStyleSheet{};
}

// 创建深色主题
JStyleSheet JStyleSheet::dark() {
    JStyleSheet s;
    s.clearColor            = JColor(0.12f, 0.12f, 0.14f, 1.0f);
    s.containerBackground   = JColor(0.18f, 0.18f, 0.20f, 1.0f);
    s.buttonBackground      = JColor(0.25f, 0.50f, 0.85f, 1.0f);
    s.buttonPrimaryBackground = JColor(0.20f, 0.40f, 0.75f, 1.0f);
    s.buttonHoverBackground = JColor(1.0f, 0.85f, 0.0f, 1.0f);
    s.buttonTextColor       = JColor(0.95f, 0.95f, 0.95f, 1.0f);
    s.textColor             = JColor(0.90f, 0.90f, 0.90f, 1.0f);
    s.inputBackground       = JColor(0.22f, 0.22f, 0.24f, 1.0f);
    s.inputBorderColor      = JColor(0.40f, 0.40f, 0.45f, 1.0f);
    s.inputTextColor        = JColor(0.90f, 0.90f, 0.90f, 1.0f);
    s.cardBackground        = JColor(0.22f, 0.22f, 0.24f, 1.0f);
    s.cardBorderColor       = JColor(0.35f, 0.35f, 0.40f, 1.0f);
    s.cardShadowColor       = JColor(0.0f, 0.0f, 0.0f, 0.12f);
    return s;
}

} // namespace jaether
