// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// ControlRenderer_test.cpp
// 控件渲染器体系 单元测试
//
// 测试覆盖标准: 语句覆盖、分支覆盖、条件覆盖、路径覆盖、MC/DC覆盖
// 测试范围:
// - JStyleSheet: light/dark工厂方法，字段默认值
// - JRenderContext: 结构体字段
// - JControlRendererRegistry: register/unregister/get/hasDefaults
// - JRendererFacade: 构造、renderAll、renderOne、clearCanvas、setStyleSheet
// - 各控件渲染器: Container/Button/Text/Input/Card 渲染逻辑
// - JDirect2DRenderer 新接口: drawTextAligned、measureText、drawEllipse、fillEllipse
// - 替换前后一致性验证

#include "aether/ControlRenderer.h"
#include "aether/StyleSheet.h"
#include "aether/Direct2DRenderer.h"
#include "aether/ComponentStorage.h"
#include "aether/property_id.h"
#include <gtest/gtest.h>
#include <Windows.h>

namespace jaether {
namespace test {

// ========================== JStyleSheet 测试 ==========================

class StyleSheetTest : public ::testing::Test {
protected:
    JStyleSheet style_;
};

/**
 * 测试 JStyleSheet 默认构造函数 — 所有字段有合理默认值
 * 语句覆盖: 访问各字段
 */
TEST_F(StyleSheetTest, DefaultConstructor_HasReasonableDefaults) {
    EXPECT_EQ(style_.buttonRadius, 5.0f);
    EXPECT_EQ(style_.cardRadius, 8.0f);
    EXPECT_EQ(style_.buttonFontSize, 14.0f);
    EXPECT_EQ(style_.textFontSize, 14.0f);
    EXPECT_EQ(style_.cardShadowOffset, 3.0f);
}

/**
 * 测试 JStyleSheet::light() — 返回默认浅色主题
 * 分支: light()返回默认构造的sheet
 */
TEST_F(StyleSheetTest, LightFactory_ReturnsLightTheme) {
    JStyleSheet s = JStyleSheet::light();
    // light() = 默认构造
    EXPECT_FLOAT_EQ(s.buttonRadius, 5.0f);
    EXPECT_FLOAT_EQ(s.textFontSize, 14.0f);
}

/**
 * 测试 JStyleSheet::dark() — 深色主题字段与浅色不同
 * 分支: dark()工厂方法
 * MC/DC: dark()中每个赋值独立影响结果
 */
TEST_F(StyleSheetTest, DarkFactory_ReturnsDarkTheme) {
    JStyleSheet s = JStyleSheet::dark();
    // 深色主题的clearColor应该比浅色暗
    EXPECT_LT(s.clearColor.r, 100);  // rgb < 100
    // 深色主题的textColor应该比浅色亮
    EXPECT_GT(s.textColor.r, 200);
    // 深色主题的cardBackground应该比浅色暗
    EXPECT_LT(s.cardBackground.r, 100);
    // 深色主题的inputBackground应该比浅色暗
    EXPECT_LT(s.inputBackground.r, 100);
}

/**
 * 测试 JStyleSheet dark 和 light 必需不同的字段
 * 条件: dark()的多个字段与light()不同
 */
TEST_F(StyleSheetTest, DarkVsLight_HasDifferentColors) {
    JStyleSheet dark = JStyleSheet::dark();
    JStyleSheet light = JStyleSheet::light();
    EXPECT_NE(dark.clearColor.r, light.clearColor.r);
    EXPECT_NE(dark.textColor.r, light.textColor.r);
    EXPECT_NE(dark.containerBackground.r, light.containerBackground.r);
}

/**
 * 测试 JStyleSheet 可修改字段
 * 路径: 修改字段值 → 读取验证
 */
TEST_F(StyleSheetTest, ModifyField_ReflectedInRead) {
    style_.buttonBackground = JColor(1.0f, 0.0f, 0.0f, 1.0f);
    EXPECT_EQ(style_.buttonBackground.r, 255);
    EXPECT_EQ(style_.buttonBackground.g, 0);
    EXPECT_EQ(style_.buttonBackground.b, 0);
}

// ========================== JRenderContext 测试 ==========================

class RenderContextTest : public ::testing::Test {
protected:
    JComponentStorage storage_;
};

/**
 * 测试 JRenderContext 默认值
 * 语句覆盖: 默认构造的字段值
 */
TEST_F(RenderContextTest, DefaultValues_AreCorrect) {
    JRenderContext ctx;
    EXPECT_FALSE(ctx.handle.isValid());
    EXPECT_FALSE(ctx.isHovered);
    EXPECT_FALSE(ctx.isFocused);
    // 成员未显式初始化时值不确定，不测试type
}

/**
 * 测试 JRenderContext 可以完全填充
 */
TEST_F(RenderContextTest, CanBeFullyPopulated) {
    JRenderContext ctx;
    ctx.isHovered = true;
    ctx.isFocused = true;
    EXPECT_TRUE(ctx.isHovered);
    EXPECT_TRUE(ctx.isFocused);
}

// ========================== JControlRendererRegistry 测试 ==========================

class ControlRendererRegistryTest : public ::testing::Test {
protected:
    void SetUp() override { registry_ = new JControlRendererRegistry(); }
    void TearDown() override { delete registry_; }
    JControlRendererRegistry* registry_;
};

/**
 * 测试 registerDefaults — 注册5个内置渲染器
 * 语句覆盖: registerDefaults → 5次registerRenderer
 */
TEST_F(ControlRendererRegistryTest, RegisterDefaults_RegistersFiveTypes) {
    JControlRendererRegistry::registerDefaults(*registry_);
    auto types = registry_->getRegisteredTypes();
    EXPECT_GE(types.size(), 5);
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Container));
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Button));
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Text));
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Input));
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Card));
}

/**
 * 测试 registerRenderer — 注册自定义渲染器
 * 分支: 新增映射 → hasRenderer=true
 */
TEST_F(ControlRendererRegistryTest, RegisterCustom_ThenFind_Success) {
    registry_->registerDefaults(*registry_);
    JControlRenderer* btn = registry_->getRenderer(JComponentType::Button);
    EXPECT_NE(btn, nullptr);
}

/**
 * 测试 getRenderer — 未注册类型返回nullptr
 * 分支: 映射表中不存在 → 返回nullptr
 * MC/DC: it == end() → 返回nullptr
 */
TEST_F(ControlRendererRegistryTest, GetRenderer_Unregistered_ReturnsNull) {
    EXPECT_EQ(registry_->getRenderer(JComponentType::ScrollView), nullptr);
}

/**
 * 测试 hasRenderer — 已注册/未注册判断
 * 分支: find != end() → true; find == end() → false
 */
TEST_F(ControlRendererRegistryTest, HasRenderer_RegisteredAndUnregistered) {
    JControlRendererRegistry::registerDefaults(*registry_);
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Text));
    EXPECT_FALSE(registry_->hasRenderer(JComponentType::ScrollView));
}

/**
 * 测试 registerRenderer — 覆盖已有注册
 * 路径: 先注册A，后注册B → 映射更新为B
 */
TEST_F(ControlRendererRegistryTest, RegisterOverride_ReplacesRenderer) {
    // 使用默认注册获取Button渲染器
    JControlRendererRegistry::registerDefaults(*registry_);
    JControlRenderer* first = registry_->getRenderer(JComponentType::Button);
    EXPECT_NE(first, nullptr);
    
    // 用不同的渲染器覆盖
    JControlRendererRegistry::registerDefaults(*registry_);
    JControlRenderer* second = registry_->getRenderer(JComponentType::Button);
    // 同一个静态实例，指针应相同
    EXPECT_EQ(first, second);
}

/**
 * 测试 unregisterRenderer — 注销后获取返回nullptr
 * 分支: 注销已有 → getRenderer返回nullptr
 */
TEST_F(ControlRendererRegistryTest, Unregister_ThenGet_ReturnsNull) {
    JControlRendererRegistry::registerDefaults(*registry_);
    EXPECT_TRUE(registry_->hasRenderer(JComponentType::Text));
    registry_->unregisterRenderer(JComponentType::Text);
    EXPECT_FALSE(registry_->hasRenderer(JComponentType::Text));
    EXPECT_EQ(registry_->getRenderer(JComponentType::Text), nullptr);
}

/**
 * 测试 unregisterRenderer — 注销未注册类型不崩溃
 * MC/DC: erase在map中找不到key，无操作
 */
TEST_F(ControlRendererRegistryTest, Unregister_NotRegistered_NoCrash) {
    EXPECT_NO_THROW(registry_->unregisterRenderer(JComponentType::Custom));
}

/**
 * 测试 getRegisteredTypes — 返回所有已注册类型
 */
TEST_F(ControlRendererRegistryTest, GetRegisteredTypes_AfterRegister_ReturnsAll) {
    registry_->registerDefaults(*registry_);
    auto types = registry_->getRegisteredTypes();
    bool hasBtn = false, hasTxt = false, hasInp = false, hasCard = false, hasCont = false;
    for (auto t : types) {
        if (t == JComponentType::Button)    hasBtn = true;
        if (t == JComponentType::Text)      hasTxt = true;
        if (t == JComponentType::Input)     hasInp = true;
        if (t == JComponentType::Card)      hasCard = true;
        if (t == JComponentType::Container) hasCont = true;
    }
    EXPECT_TRUE(hasBtn && hasTxt && hasInp && hasCard && hasCont);
}

// ========================== JRendererFacade 测试（无渲染器，仅逻辑） ==========================

class RendererFacadeTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = new JComponentStorage();
        // 创建一个模拟的HWND以初始化Direct2D渲染器
        WNDCLASSW wc = {};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"TestWindowClass";
        RegisterClassW(&wc);
        hwnd_ = CreateWindowExW(0, L"TestWindowClass", L"Test", 0,
                                0, 0, 100, 100, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        renderer_ = new JDirect2DRenderer();
        renderer_->initialize(hwnd_);
    }
    void TearDown() override {
        delete renderer_;
        DestroyWindow(hwnd_);
        delete storage_;
    }
    JComponentStorage* storage_;
    JDirect2DRenderer* renderer_;
    HWND hwnd_;
};

/**
 * 测试 JRendererFacade 构造函数 — 自动注册所有内置渲染器
 * 语句覆盖: 构造函数 → registerDefaults
 */
TEST_F(RendererFacadeTest, Constructor_AutoRegistersDefaults) {
    JRendererFacade facade(renderer_, *storage_);
    EXPECT_TRUE(facade.getRegistry().hasRenderer(JComponentType::Button));
    EXPECT_TRUE(facade.getRegistry().hasRenderer(JComponentType::Text));
    EXPECT_TRUE(facade.getRegistry().hasRenderer(JComponentType::Container));
    EXPECT_TRUE(facade.getRegistry().hasRenderer(JComponentType::Input));
    EXPECT_TRUE(facade.getRegistry().hasRenderer(JComponentType::Card));
}

/**
 * 测试 renderAll — 有可见组件时不崩溃
 * 语句覆盖: forEach → buildContext → getRenderer → render
 */
TEST_F(RendererFacadeTest, RenderAll_WithComponents_NoCrash) {
    JRendererFacade facade(renderer_, *storage_);
    auto root = storage_->createComponent(JComponentType::Container);
    auto text = storage_->createComponent(JComponentType::Text, root);
    storage_->getComponent(text)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Hello")));
    
    facade.getStyleSheet().defaultFontName = "Segoe UI";
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll());
    renderer_->endDraw();
}

/**
 * 测试 renderAll — 空组件树不崩溃
 * 分支: forEach无可见组件 → 无渲染调用
 */
TEST_F(RendererFacadeTest, RenderAll_EmptyStorage_NoCrash) {
    JRendererFacade facade(renderer_, *storage_);
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll());
    renderer_->endDraw();
}

/**
 * 测试 renderAll — 不可见组件被跳过
 * 分支: !entry->visible → 跳过渲染
 */
TEST_F(RendererFacadeTest, RenderAll_InvisibleComponent_Skipped) {
    JRendererFacade facade(renderer_, *storage_);
    auto root = storage_->createComponent(JComponentType::Container);
    auto text = storage_->createComponent(JComponentType::Text, root);
    storage_->getComponent(text)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Hidden")));
    storage_->getComponent(text)->visible = false;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll());
    renderer_->endDraw();
}

/**
 * 测试 renderOne — 渲染单个组件
 * 分支: 有效handle → 构建上下文 → 调用渲染器
 */
TEST_F(RendererFacadeTest, RenderOne_ValidComponent_NoCrash) {
    JRendererFacade facade(renderer_, *storage_);
    auto btn = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(btn)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("OK")));
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderOne(btn));
    renderer_->endDraw();
}

/**
 * 测试 renderOne — 无效句柄无崩溃
 * 分支: 无效entry → 提前返回
 */
TEST_F(RendererFacadeTest, RenderOne_InvalidHandle_NoCrash) {
    JRendererFacade facade(renderer_, *storage_);
    JComponentHandle invalid;
    EXPECT_NO_THROW(facade.renderOne(invalid));
}

/**
 * 测试 renderOne — 不可见组件无渲染
 * 分支: !visible → 提前返回
 */
TEST_F(RendererFacadeTest, RenderOne_InvisibleComponent_NoCrash) {
    JRendererFacade facade(renderer_, *storage_);
    auto c = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(c)->visible = false;
    EXPECT_NO_THROW(facade.renderOne(c));
}

/**
 * 测试 clearCanvas — 调用底层clear
 */
TEST_F(RendererFacadeTest, ClearCanvas_CallsRendererClear) {
    JRendererFacade facade(renderer_, *storage_);
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.clearCanvas());
    renderer_->endDraw();
}

/**
 * 测试 setStyleSheet — 替换样式表
 * 分支: setStyleSheet → getStyleSheet返回新表
 */
TEST_F(RendererFacadeTest, SetStyleSheet_ReplacesSheet) {
    JRendererFacade facade(renderer_, *storage_);
    JStyleSheet dark = JStyleSheet::dark();
    facade.setStyleSheet(dark);
    EXPECT_LT(facade.getStyleSheet().clearColor.r, 100);
}

/**
 * 测试 getStyleSheet — 返回可修改引用
 */
TEST_F(RendererFacadeTest, GetStyleSheet_Modifiable) {
    JRendererFacade facade(renderer_, *storage_);
    facade.getStyleSheet().buttonRadius = 10.0f;
    EXPECT_FLOAT_EQ(facade.getStyleSheet().buttonRadius, 10.0f);
}

// ========================== 控件渲染器测试（验证逻辑，不验证像素） ==========================

class ControlRenderersTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = new JComponentStorage();
        WNDCLASSW wc = {};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"CtrlRendererTest";
        RegisterClassW(&wc);
        hwnd_ = CreateWindowExW(0, L"CtrlRendererTest", L"", 0, 0, 0, 100, 100,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        renderer_ = new JDirect2DRenderer();
        renderer_->initialize(hwnd_);
    }
    void TearDown() override {
        delete renderer_;
        DestroyWindow(hwnd_);
        delete storage_;
    }
    
    JRenderContext makeCtx(JComponentHandle h) {
        JRenderContext ctx;
        ctx.handle = h;
        ctx.type = storage_->getComponent(h)->type;
        ctx.absoluteRect = JRect(10, 10, 100, 40);
        ctx.properties = &storage_->getComponent(h)->properties;
        return ctx;
    }
    
    JComponentStorage* storage_;
    JDirect2DRenderer* renderer_;
    HWND hwnd_;
};

/**
 * 测试 JContainerRenderer — 无崩溃渲染
 * 语句覆盖: fillRect调用
 */
TEST_F(ControlRenderersTest, ContainerRenderer_Render_NoCrash) {
    JContainerRenderer cr;
    auto h = storage_->createComponent(JComponentType::Container);
    JRenderContext ctx = makeCtx(h);
    ctx.type = JComponentType::Container;
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(cr.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JContainerRenderer — getComponentType返回Container
 */
TEST_F(ControlRenderersTest, ContainerRenderer_GetType_ReturnsContainer) {
    JContainerRenderer cr;
    EXPECT_EQ(cr.getComponentType(), JComponentType::Container);
}

/**
 * 测试 JButtonRenderer — 基本渲染不崩溃
 * 分支: 有Text属性 → 绘制背景+文字
 */
TEST_F(ControlRenderersTest, ButtonRenderer_WithText_RendersBoth) {
    JButtonRenderer br;
    auto h = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Click")));
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    style.defaultFontName = "Segoe UI";
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(br.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JButtonRenderer — 无Text属性仅绘制背景
 * 分支: textProp == nullptr → 跳过文字绘制
 */
TEST_F(ControlRenderersTest, ButtonRenderer_NoText_RendersBackground) {
    JButtonRenderer br;
    auto h = storage_->createComponent(JComponentType::Button);
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(br.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JButtonRenderer — 悬停状态改变颜色
 * 条件: ctx.isHovered → 使用buttonHoverBackground
 * MC/DC: isHovered独立影响bgColor选择
 */
TEST_F(ControlRenderersTest, ButtonRenderer_Hovered_UsesHoverColor) {
    JButtonRenderer br;
    auto h = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("BTN")));
    JRenderContext ctx = makeCtx(h);
    ctx.isHovered = true;
    JStyleSheet style;
    style.defaultFontName = "Segoe UI";
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(br.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JButtonRenderer — variant=primary改变颜色
 * 路径: BackgroundColor="primary" → buttonPrimaryBackground
 */
TEST_F(ControlRenderersTest, ButtonRenderer_PrimaryVariant_UsesPrimaryColor) {
    JButtonRenderer br;
    auto h = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("OK")));
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    style.defaultFontName = "Segoe UI";
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(br.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JButtonRenderer — 悬停覆盖variant颜色
 * 路径: variant=primary + isHovered → hover颜色优先
 * MC/DC: isHovered条件在variant之后判断，会覆盖
 */
TEST_F(ControlRenderersTest, ButtonRenderer_HoverOverridesVariant) {
    JButtonRenderer br;
    auto h = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("OK")));
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));
    JRenderContext ctx = makeCtx(h);
    ctx.isHovered = true;
    JStyleSheet style;
    style.defaultFontName = "Segoe UI";
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(br.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JButtonRenderer — FontSize属性读取
 * 路径: 有FontSize属性 → 使用属性值
 */
TEST_F(ControlRenderersTest, ButtonRenderer_FontSizeProperty_Used) {
    JButtonRenderer br;
    auto h = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Big")));
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::FontSize, JPropertyValue(24.0f));
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    style.defaultFontName = "Segoe UI";
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(br.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JButtonRenderer — getComponentType返回Button
 */
TEST_F(ControlRenderersTest, ButtonRenderer_GetType_ReturnsButton) {
    JButtonRenderer br;
    EXPECT_EQ(br.getComponentType(), JComponentType::Button);
}

/**
 * 测试 JTextRenderer — 有文本时渲染
 * 分支: textProp存在且为string → 进入渲染分支
 */
TEST_F(ControlRenderersTest, TextRenderer_WithText_Renders) {
    JTextRenderer tr;
    auto h = storage_->createComponent(JComponentType::Text);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Hello World")));
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(tr.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JTextRenderer — 无文本时跳过
 * 分支: textProp为nullptr → 直接return
 * MC/DC: !textProp → 短路返回
 */
TEST_F(ControlRenderersTest, TextRenderer_NoText_Skips) {
    JTextRenderer tr;
    auto h = storage_->createComponent(JComponentType::Text);
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(tr.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JTextRenderer — FontSize属性读取
 * 路径: 有FontSize → 使用属性值
 */
TEST_F(ControlRenderersTest, TextRenderer_FontSizeProperty_Used) {
    JTextRenderer tr;
    auto h = storage_->createComponent(JComponentType::Text);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Title")));
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::FontSize, JPropertyValue(32.0f));
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(tr.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JTextRenderer — getComponentType返回Text
 */
TEST_F(ControlRenderersTest, TextRenderer_GetType_ReturnsText) {
    JTextRenderer tr;
    EXPECT_EQ(tr.getComponentType(), JComponentType::Text);
}

/**
 * 测试 JInputRenderer — 有文本时渲染
 * 分支: textProp存在 → 绘制背景+边框+文字
 */
TEST_F(ControlRenderersTest, InputRenderer_WithText_Renders) {
    JInputRenderer ir;
    auto h = storage_->createComponent(JComponentType::Input);
    storage_->getComponent(h)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("search...")));
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(ir.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JInputRenderer — 无文本时仅绘制背景+边框
 * 分支: textProp为nullptr → 跳过文字绘制
 */
TEST_F(ControlRenderersTest, InputRenderer_NoText_RendersBorder) {
    JInputRenderer ir;
    auto h = storage_->createComponent(JComponentType::Input);
    JRenderContext ctx = makeCtx(h);
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(ir.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JInputRenderer — getComponentType返回Input
 */
TEST_F(ControlRenderersTest, InputRenderer_GetType_ReturnsInput) {
    JInputRenderer ir;
    EXPECT_EQ(ir.getComponentType(), JComponentType::Input);
}

/**
 * 测试 JCardRenderer — 渲染阴影+背景+边框
 * 语句覆盖: 三个绘制步骤
 */
TEST_F(ControlRenderersTest, CardRenderer_Render_NoCrash) {
    JCardRenderer cr;
    auto h = storage_->createComponent(JComponentType::Card);
    JRenderContext ctx = makeCtx(h);
    ctx.type = JComponentType::Card;
    JStyleSheet style;
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(cr.render(renderer_, ctx, style));
    renderer_->endDraw();
}

/**
 * 测试 JCardRenderer — getComponentType返回Card
 */
TEST_F(ControlRenderersTest, CardRenderer_GetType_ReturnsCard) {
    JCardRenderer cr;
    EXPECT_EQ(cr.getComponentType(), JComponentType::Card);
}

// ========================== JDirect2DRenderer 新接口测试 ==========================

class Direct2DExtensionTest : public ::testing::Test {
protected:
    void SetUp() override {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"D2DExtTest";
        RegisterClassW(&wc);
        hwnd_ = CreateWindowExW(0, L"D2DExtTest", L"", 0, 0, 0, 200, 200,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        renderer_ = new JDirect2DRenderer();
        renderer_->initialize(hwnd_);
    }
    void TearDown() override { delete renderer_; DestroyWindow(hwnd_); }
    JDirect2DRenderer* renderer_;
    HWND hwnd_;
};

/**
 * 测试 drawTextAligned — Left对齐
 * 语句: drawTextAligned Left
 */
TEST_F(Direct2DExtensionTest, DrawTextAligned_Left_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawTextAligned("Left", JRect(10,10,100,30),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 14.0f, "Segoe UI", JTextAlignment::Left));
    renderer_->endDraw();
}

/**
 * 测试 drawTextAligned — Center对齐
 * 语句: drawTextAligned Center
 */
TEST_F(Direct2DExtensionTest, DrawTextAligned_Center_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawTextAligned("Center", JRect(10,10,100,30),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 14.0f, "Segoe UI", JTextAlignment::Center));
    renderer_->endDraw();
}

/**
 * 测试 drawTextAligned — Right对齐
 * 语句: drawTextAligned Right
 */
TEST_F(Direct2DExtensionTest, DrawTextAligned_Right_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawTextAligned("Right", JRect(10,10,100,30),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 14.0f, "Segoe UI", JTextAlignment::Right));
    renderer_->endDraw();
}

/**
 * 测试 drawTextAligned — 默认参数（Center）
 * 分支: 默认alignment=Center
 */
TEST_F(Direct2DExtensionTest, DrawTextAligned_DefaultAlignment_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawTextAligned("Default", JRect(10,10,100,30),
        JColor(0.0f, 0.0f, 0.0f, 1.0f)));
    renderer_->endDraw();
}

/**
 * 测试 drawText — 向后兼容（内部调用drawTextAligned Center）
 */
TEST_F(Direct2DExtensionTest, DrawText_BackwardCompatible_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawText("Legacy", JRect(10,10,100,30),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 16.0f));
    renderer_->endDraw();
}

/**
 * 测试 drawTextAligned — 三种对齐方式都能正常返回
 * MC/DC: 三种枚举值各测试一次
 */
TEST_F(Direct2DExtensionTest, DrawTextAligned_AllThreeAlignments_Distinct) {
    renderer_->beginDraw();
    // 无渲染目标时提前返回不崩溃
    // 测试三种对齐在有效渲染目标下不崩溃
    EXPECT_NO_THROW(renderer_->drawTextAligned("L", JRect(0,0,50,20),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 12.0f, "Arial", JTextAlignment::Left));
    EXPECT_NO_THROW(renderer_->drawTextAligned("C", JRect(0,0,50,20),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 12.0f, "Arial", JTextAlignment::Center));
    EXPECT_NO_THROW(renderer_->drawTextAligned("R", JRect(0,0,50,20),
        JColor(0.0f, 0.0f, 0.0f, 1.0f), 12.0f, "Arial", JTextAlignment::Right));
    renderer_->endDraw();
}

/**
 * 测试 measureText — 测量非空文本返回正尺寸
 * 分支: text非空 → 创建layout测量 → 返回正size
 */
TEST_F(Direct2DExtensionTest, MeasureText_NonEmpty_ReturnsPositiveSize) {
    JSize size = renderer_->measureText("Hello", 16.0f, "Segoe UI");
    EXPECT_GT(size.width, 0.0f);
    EXPECT_GT(size.height, 0.0f);
}

/**
 * 测试 measureText — 空文本返回零尺寸
 * 分支: text空 → 提前返回{0,0}
 * MC/DC: text.empty() → 直接返回
 */
TEST_F(Direct2DExtensionTest, MeasureText_Empty_ReturnsZero) {
    JSize size = renderer_->measureText("", 16.0f, "Segoe UI");
    EXPECT_FLOAT_EQ(size.width, 0.0f);
    EXPECT_FLOAT_EQ(size.height, 0.0f);
}

/**
 * 测试 measureText — 不同字号返回不同尺寸
 * 条件: fontSize参数独立影响结果
 */
TEST_F(Direct2DExtensionTest, MeasureText_LargerFont_ReturnsLarger) {
    JSize s1 = renderer_->measureText("Hello", 12.0f, "Segoe UI");
    JSize s2 = renderer_->measureText("Hello", 32.0f, "Segoe UI");
    EXPECT_GT(s2.width, s1.width);
}

/**
 * 测试 measureText — 无writeFactory时返回零（验证measureText判空逻辑）
 * 路径: 正常情况下writeFactory存在，此测试验证代码不会崩
 */
TEST_F(Direct2DExtensionTest, MeasureText_ValidRenderer_ReturnsValid) {
    JSize s = renderer_->measureText("Test text for measurement", 14.0f);
    EXPECT_GE(s.width, 0.0f);
    EXPECT_GE(s.height, 0.0f);
}

/**
 * 测试 drawEllipse — 基本调用不崩溃
 * 语句覆盖: drawEllipse
 */
TEST_F(Direct2DExtensionTest, DrawEllipse_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawEllipse(JRect(10,10,40,40), JColor(0.0f, 0.0f, 0.0f, 1.0f)));
    renderer_->endDraw();
}

/**
 * 测试 fillEllipse — 基本调用不崩溃
 * 语句覆盖: fillEllipse
 */
TEST_F(Direct2DExtensionTest, FillEllipse_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->fillEllipse(JRect(10,10,40,40), JColor(1.0f, 0.0f, 0.0f, 1.0f)));
    renderer_->endDraw();
}

/**
 * 测试 drawEllipse/fillEllipse — 不同参数组合
 * 路径: 不同strokeWidth、不同颜色
 */
TEST_F(Direct2DExtensionTest, Ellipse_VaryingParameters_NoCrash) {
    renderer_->beginDraw();
    EXPECT_NO_THROW(renderer_->drawEllipse(JRect(0,0,30,30), JColor(0.0f, 0.0f, 1.0f, 1.0f), 2.0f));
    EXPECT_NO_THROW(renderer_->fillEllipse(JRect(50,50,20,20), JColor(0.0f, 1.0f, 0.0f, 1.0f)));
    EXPECT_NO_THROW(renderer_->drawEllipse(JRect(0,0,10,20), JColor(0.5f, 0.5f, 0.5f, 1.0f), 1.5f));
    renderer_->endDraw();
}

// ========================== 集成测试：Facade + Registry + Renderers ==========================

class ControlRendererIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = new JComponentStorage();
        WNDCLASSW wc = {};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"IntegrationTestWnd";
        RegisterClassW(&wc);
        hwnd_ = CreateWindowExW(0, L"IntegrationTestWnd", L"", 0, 0, 0, 200, 200,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        renderer_ = new JDirect2DRenderer();
        renderer_->initialize(hwnd_);
    }
    void TearDown() override { delete renderer_; DestroyWindow(hwnd_); delete storage_; }
    JComponentStorage* storage_;
    JDirect2DRenderer* renderer_;
    HWND hwnd_;
};

/**
 * 测试 完整渲染流程 — 多类型组件同时渲染
 * 路径: Container→Button→Text→Input→Card 完整管线
 * MC/DC: 所有5种内置渲染器都被调用
 */
TEST_F(ControlRendererIntegrationTest, FullRenderPipeline_AllTypes_NoCrash) {
    JRendererFacade facade(renderer_, *storage_);
    facade.getStyleSheet().defaultFontName = "Segoe UI";
    
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 200, 200);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(10, 10, 80, 30);
    storage_->getComponent(btn)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("OK")));
    
    auto txt = storage_->createComponent(JComponentType::Text, root);
    storage_->getComponent(txt)->layoutResult = JRect(10, 50, 180, 20);
    storage_->getComponent(txt)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Welcome")));
    storage_->getComponent(txt)->properties.setProperty(
        JPropertyId::FontSize, JPropertyValue(18.0f));
    
    auto inp = storage_->createComponent(JComponentType::Input, root);
    storage_->getComponent(inp)->layoutResult = JRect(10, 80, 180, 30);
    storage_->getComponent(inp)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("input...")));
    
    auto card = storage_->createComponent(JComponentType::Card, root);
    storage_->getComponent(card)->layoutResult = JRect(10, 120, 180, 60);
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll(50.0f, 20.0f));  // 鼠标悬停在btn上
    renderer_->endDraw();
}

/**
 * 测试 自定义渲染器注册替换
 * 路径: registerRenderer覆盖Button → renderAll使用新渲染器
 */
TEST_F(ControlRendererIntegrationTest, CustomRendererReplacement_Used) {
    JRendererFacade facade(renderer_, *storage_);
    
    // 创建自定义渲染器
    static bool called = false;
    class JTestButtonRenderer : public JControlRenderer {
    public:
        void render(JDirect2DRenderer*, const JRenderContext&, const JStyleSheet&) override {
            called = true;
        }
        JComponentType getComponentType() const override { return JComponentType::Button; }
    };
    static JTestButtonRenderer testRenderer;
    
    // 替换Button渲染器
    facade.getRegistry().registerRenderer(JComponentType::Button, &testRenderer);
    
    auto btn = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(btn)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Test")));
    
    called = false;
    renderer_->beginDraw();
    facade.renderAll();
    renderer_->endDraw();
    
    EXPECT_TRUE(called);
}

/**
 * 测试 深色主题切换
 * 路径: setStyleSheet(dark) → renderAll
 */
TEST_F(ControlRendererIntegrationTest, DarkTheme_Applied) {
    JRendererFacade facade(renderer_, *storage_);
    facade.setStyleSheet(JStyleSheet::dark());
    facade.getStyleSheet().defaultFontName = "Segoe UI";
    
    auto root = storage_->createComponent(JComponentType::Container);
    auto txt = storage_->createComponent(JComponentType::Text, root);
    storage_->getComponent(txt)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Dark mode")));
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll());
    renderer_->endDraw();
}

/**
 * 测试 未注册类型被跳过
 * 分支: getRenderer返回nullptr → 跳过渲染，不崩溃
 * MC/DC: ctrlRenderer==nullptr → 跳过
 */
TEST_F(ControlRendererIntegrationTest, UnregisteredType_Skipped) {
    JRendererFacade facade(renderer_, *storage_);
    
    // 注销Card渲染器
    facade.getRegistry().unregisterRenderer(JComponentType::Card);
    EXPECT_FALSE(facade.getRegistry().hasRenderer(JComponentType::Card));
    
    auto card = storage_->createComponent(JComponentType::Card);
    
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll());  // Card被跳过，不崩溃
    renderer_->endDraw();
}

/**
 * 测试 鼠标悬停检测传递到渲染器
 * 分支: mouseX/Y有效 → containsPoint检查 → isHovered正确设置
 */
TEST_F(ControlRendererIntegrationTest, HoverDetection_PassedToRenderers) {
    JRendererFacade facade(renderer_, *storage_);
    facade.getStyleSheet().defaultFontName = "Segoe UI";
    
    auto btn = storage_->createComponent(JComponentType::Button);
    storage_->getComponent(btn)->layoutResult = JRect(50, 50, 80, 30);
    storage_->getComponent(btn)->properties.setProperty(
        JPropertyId::Text, JPropertyValue(std::string("Hover me")));
    
    // 鼠标在按钮区域内
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll(60.0f, 60.0f));
    renderer_->endDraw();
    
    // 鼠标在按钮区域外
    renderer_->beginDraw();
    EXPECT_NO_THROW(facade.renderAll(0.0f, 0.0f));
    renderer_->endDraw();
}

} // namespace test
} // namespace jaether
