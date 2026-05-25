// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// ControlRenderer.cpp
// 控件渲染器模块 - 应用层统一控件渲染体系
//
// 功能:
// - JContainerRenderer/JButtonRenderer/JTextRenderer/JInputRenderer/JCardRenderer 内置渲染器实现
// - JControlRendererRegistry 渲染器注册中心
// - JRendererFacade 统一渲染门面

#include "aether/ControlRenderer.h"
#include "aether/property_id.h"

namespace jaether {

// ==================== JContainerRenderer ====================

void JContainerRenderer::render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                                 const JStyleSheet& style) {
    renderer->fillRect(ctx.absoluteRect, style.containerBackground);
}

JComponentType JContainerRenderer::getComponentType() const {
    return JComponentType::Container;
}

// ==================== JButtonRenderer ====================

void JButtonRenderer::render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                              const JStyleSheet& style) {
    // 步骤1：确定按钮背景色
    JColor bgColor = style.buttonBackground;
    
    // 从属性读取 variant 字段（BackgroundColor 属性存储 variant 字符串）
    auto* bgProp = ctx.properties->getProperty(JPropertyId::BackgroundColor);
    if (bgProp && bgProp->is<std::string>()) {
        const std::string& variant = bgProp->get<std::string>();
        if (variant == "primary") {
            bgColor = style.buttonPrimaryBackground;
        }
    }
    
    // 悬停状态覆盖背景色
    if (ctx.isHovered) {
        bgColor = style.buttonHoverBackground;
    }
    
    // 步骤2：绘制圆角矩形背景
    renderer->fillRoundedRect(ctx.absoluteRect, style.buttonRadius,
                              style.buttonRadius, bgColor);
    
    // 步骤3：绘制按钮文本（居中）
    auto* textProp = ctx.properties->getProperty(JPropertyId::Text);
    if (textProp && textProp->is<std::string>()) {
        // 字号：优先从属性FontSize读取
        float fontSize = style.buttonFontSize;
        auto* fsProp = ctx.properties->getProperty(JPropertyId::FontSize);
        if (fsProp) {
            if (fsProp->is<float>()) fontSize = fsProp->get<float>();
            else if (fsProp->is<int>()) fontSize = static_cast<float>(fsProp->get<int>());
        }
        
        renderer->drawTextAligned(textProp->get<std::string>(), ctx.absoluteRect,
                                  style.buttonTextColor, fontSize,
                                  style.defaultFontName, JTextAlignment::Center);
    }
}

JComponentType JButtonRenderer::getComponentType() const {
    return JComponentType::Button;
}

// ==================== JTextRenderer ====================

void JTextRenderer::render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                            const JStyleSheet& style) {
    // 读取文本属性
    auto* textProp = ctx.properties->getProperty(JPropertyId::Text);
    if (!textProp || !textProp->is<std::string>()) return;
    
    // 字号：优先从属性FontSize读取，否则用样式默认值
    float fontSize = style.textFontSize;
    auto* fsProp = ctx.properties->getProperty(JPropertyId::FontSize);
    if (fsProp) {
        if (fsProp->is<float>()) fontSize = fsProp->get<float>();
        else if (fsProp->is<int>()) fontSize = static_cast<float>(fsProp->get<int>());
    }
    
    // 颜色：使用样式默认文本颜色
    JColor textColor = style.textColor;
    
    // 左对齐绘制文本（方便阅读型内容）
    renderer->drawTextAligned(textProp->get<std::string>(), ctx.absoluteRect,
                              textColor, fontSize,
                              style.defaultFontName, JTextAlignment::Left);
}

JComponentType JTextRenderer::getComponentType() const {
    return JComponentType::Text;
}

// ==================== JInputRenderer ====================

void JInputRenderer::render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                             const JStyleSheet& style) {
    // 步骤1：白色背景填充
    renderer->fillRect(ctx.absoluteRect, style.inputBackground);
    
    // 步骤2：灰色边框
    renderer->drawRect(ctx.absoluteRect, style.inputBorderColor, style.inputBorderWidth);
    
    // 步骤3：左对齐绘制输入文本（带内边距）
    auto* textProp = ctx.properties->getProperty(JPropertyId::Text);
    if (textProp && textProp->is<std::string>()) {
        // 计算带内边距的文本区域
        JRect textRect = ctx.absoluteRect;
        textRect.x += style.inputTextPadding;
        textRect.width -= style.inputTextPadding * 2.0f;
        
        // 字号
        float fontSize = style.inputFontSize;
        auto* fsProp = ctx.properties->getProperty(JPropertyId::FontSize);
        if (fsProp) {
            if (fsProp->is<float>()) fontSize = fsProp->get<float>();
            else if (fsProp->is<int>()) fontSize = static_cast<float>(fsProp->get<int>());
        }
        
        // 左对齐
        renderer->drawTextAligned(textProp->get<std::string>(), textRect,
                                  style.inputTextColor, fontSize,
                                  style.defaultFontName, JTextAlignment::Left);
    }
}

JComponentType JInputRenderer::getComponentType() const {
    return JComponentType::Input;
}

// ==================== JCardRenderer ====================

void JCardRenderer::render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                            const JStyleSheet& style) {
    // 步骤1：绘制阴影（偏移的半透明深色矩形）
    JRect shadowRect = ctx.absoluteRect;
    shadowRect.x += style.cardShadowOffset;
    shadowRect.y += style.cardShadowOffset;
    renderer->fillRoundedRect(shadowRect, style.cardRadius, style.cardRadius,
                              style.cardShadowColor);
    
    // 步骤2：白色圆角背景
    renderer->fillRoundedRect(ctx.absoluteRect, style.cardRadius, style.cardRadius,
                              style.cardBackground);
    
    // 步骤3：灰色圆角边框
    renderer->drawRoundedRect(ctx.absoluteRect, style.cardRadius, style.cardRadius,
                              style.cardBorderColor, style.cardBorderWidth);
}

JComponentType JCardRenderer::getComponentType() const {
    return JComponentType::Card;
}

// ==================== JControlRendererRegistry ====================

JControlRendererRegistry::JControlRendererRegistry() {
}

JControlRendererRegistry::~JControlRendererRegistry() {
}

void JControlRendererRegistry::registerRenderer(JComponentType type, JControlRenderer* renderer) {
    map_[type] = renderer;
}

void JControlRendererRegistry::unregisterRenderer(JComponentType type) {
    map_.erase(type);
}

JControlRenderer* JControlRendererRegistry::getRenderer(JComponentType type) const {
    auto it = map_.find(type);
    if (it != map_.end()) {
        return it->second;
    }
    return nullptr;
}

bool JControlRendererRegistry::hasRenderer(JComponentType type) const {
    return map_.find(type) != map_.end();
}

std::vector<JComponentType> JControlRendererRegistry::getRegisteredTypes() const {
    std::vector<JComponentType> result;
    result.reserve(map_.size());
    for (const auto& kv : map_) {
        result.push_back(kv.first);
    }
    return result;
}

void JControlRendererRegistry::registerDefaults(JControlRendererRegistry& registry) {
    // 使用静态局部变量，生命周期与进程一致
    static JContainerRenderer s_containerRenderer;
    static JButtonRenderer    s_buttonRenderer;
    static JTextRenderer      s_textRenderer;
    static JInputRenderer     s_inputRenderer;
    static JCardRenderer      s_cardRenderer;

    registry.registerRenderer(JComponentType::Container, &s_containerRenderer);
    registry.registerRenderer(JComponentType::Button,    &s_buttonRenderer);
    registry.registerRenderer(JComponentType::Text,      &s_textRenderer);
    registry.registerRenderer(JComponentType::Input,     &s_inputRenderer);
    registry.registerRenderer(JComponentType::Card,      &s_cardRenderer);
}

// ==================== JRendererFacade ====================

JRendererFacade::JRendererFacade(JDirect2DRenderer* renderer, JComponentStorage& storage)
    : renderer_(renderer)
    , storage_(storage) {
    // 自动注册所有内置渲染器
    JControlRendererRegistry::registerDefaults(registry_);
}

void JRendererFacade::renderAll(float mouseX, float mouseY) {
    // 遍历所有活跃的可见组件
    storage_.forEach([this, mouseX, mouseY](JComponentHandle handle) {
        auto* entry = storage_.getComponent(handle);
        if (!entry || !entry->visible) return;
        
        // 构建渲染上下文
        JRenderContext ctx = buildContext(handle, mouseX, mouseY);
        
        // 查找对应的控件渲染器
        JControlRenderer* ctrlRenderer = registry_.getRenderer(entry->type);
        if (!ctrlRenderer) return;  // 未注册渲染器的控件类型，跳过
        
        // 执行渲染
        ctrlRenderer->render(renderer_, ctx, styleSheet_);
    });
}

void JRendererFacade::renderOne(JComponentHandle handle, float mouseX, float mouseY) {
    auto* entry = storage_.getComponent(handle);
    if (!entry || !entry->visible) return;
    
    // 构建渲染上下文（含绝对坐标、悬停检测）
    JRenderContext ctx = buildContext(handle, mouseX, mouseY);
    
    // 查找并调用渲染器
    JControlRenderer* ctrlRenderer = registry_.getRenderer(entry->type);
    if (!ctrlRenderer) return;
    
    ctrlRenderer->render(renderer_, ctx, styleSheet_);
}

void JRendererFacade::clearCanvas() {
    renderer_->clear(styleSheet_.clearColor);
}

JRenderContext JRendererFacade::buildContext(JComponentHandle handle, 
                                              float mouseX, float mouseY) {
    JRenderContext ctx;
    ctx.handle = handle;
    
    auto* entry = storage_.getComponent(handle);
    if (!entry) return ctx;
    
    ctx.type = entry->type;
    // 使用绝对窗口坐标作为渲染矩形
    ctx.absoluteRect = storage_.getAbsoluteBounds(handle);
    ctx.properties = &entry->properties;
    
    // 悬停检测：使用containsPoint判断鼠标是否在组件区域
    ctx.isHovered = storage_.containsPoint(handle, mouseX, mouseY);
    
    return ctx;
}

} // namespace jaether
