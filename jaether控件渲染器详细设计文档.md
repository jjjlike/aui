# jaether 应用层控件渲染器详细设计文档

> **版本**：1.0.0  
> **目标**：设计一个统一的应用层控件渲染体系，替代当前分散的控件渲染代码，提供一致的默认样式、良好的扩展性和可定制性。  
> **原则**：分层解耦、策略可替换、默认即美观、最小侵入。

---

## 目录

1. [问题分析：现有渲染代码的痛点](#1-问题分析现有渲染代码的痛点)
2. [总体架构设计](#2-总体架构设计)
3. [JDirect2DRenderer 扩展接口设计](#3-jdirect2drenderer-扩展接口设计)
4. [JControlRenderer 基类与子类设计](#4-jcontrolrenderer-基类与子类设计)
5. [JStyleSheet — 统一默认样式系统](#5-jstylesheet--统一默认样式系统)
6. [JRenderContext — 渲染上下文](#6-jrendercontext--渲染上下文)
7. [JControlRendererRegistry — 渲染器注册中心](#7-jcontrolrendererregistry--渲染器注册中心)
8. [JRendererFacade — 对外的统一渲染门面](#8-jrendererfacade--对外的统一渲染门面)
9. [替换现有分散代码的方案](#9-替换现有分散代码的方案)
10. [扩展性设计：用户自定义渲染器](#10-扩展性设计用户自定义渲染器)
11. [新增文件与修改文件清单](#11-新增文件与修改文件清单)
12. [实现计划](#12-实现计划)

---

## 1. 问题分析：现有渲染代码的痛点

### 1.1 当前散布位置

控件渲染逻辑分散在两处，代码重复且风格不一致：

| 位置 | 文件 | 渲染的控件 | 代码量 |
|------|------|-----------|--------|
| AetherApplication | `src/AetherApplication.cpp` L345-390 | Container, Button, Text | ~45行 |
| TodoAppWindow | `todo-app/main.cpp` L340-435 | Container, Button, Text, Input, Card | ~95行 |

### 1.2 具体问题

**问题1：代码重复**。Container、Button、Text 的渲染逻辑在两处几乎一样但又不完全一样：
```
AetherApplication:  Button → 蓝色圆角矩形 + 白色文字
TodoAppWindow:      Button → 红色圆角矩形 + 悬停变黄 + 白色文字
```

**问题2：样式硬编码**。颜色、字号、圆角半径等直接写死在 switch/case 中，无法统一调整：
```cpp
// AetherApplication.cpp
renderer_->fillRoundedRect(rect, 5.0f, 5.0f, JColor(0.2f, 0.5f, 0.9f, 1.0f));  // 蓝色
renderer_->drawText(..., JColor(1.0f, 1.0f, 1.0f, 1.0f), 16.0f);                // 16号字

// todo-app/main.cpp
renderer_->fillRoundedRect(rect, 5.0f, 5.0f, btnColor);  // btnColor 动态计算
renderer_->drawText(..., JColor(1.0f, 1.0f, 1.0f, 1.0f), 16.0f);
```

**问题3：无扩展性**。新增控件类型（如 Slider、CheckBox）需要在每个应用层手动添加 case 分支。

**问题4：状态逻辑杂糅**。悬停检测（`storage.containsPoint(handle, mouseX, mouseY)`）混在渲染代码中，违反单一职责。

**问题5：无字体大小自适应**。Text 组件设置的 `FontSize` 属性被忽略，两处都硬编码字体大小（16.0f / 20.0f），未能从 `JPropertyId::FontSize` 读取。

**问题6：Card 只在 todo-app 有渲染**。AetherApplication 中 Card 走 default 分支，导致不显示。

### 1.3 期望效果

```
渲染前（现状）:
  应用A → switch/case → 直接调 JDirect2DRenderer API
  应用B → switch/case → 直接调 JDirect2DRenderer API
  样式: 各自硬编码，不统一

渲染后（目标）:
  应用A ─┐
  应用B ─┤→ JRendererFacade::render(entry) → JControlRenderer → JDirect2DRenderer
          └→ 统一默认样式（可覆盖）
```

---

## 2. 总体架构设计

### 2.1 四层架构

```
┌──────────────────────────────────────────────────────────────────┐
│                    应用层（AetherApplication / TodoAppWindow）    │
│  render() { facade_.renderAll(storage, mouseX, mouseY); }        │
└──────────────────────────┬───────────────────────────────────────┘
                           │
┌──────────────────────────┼───────────────────────────────────────┐
│              JRendererFacade（渲染门面）                           │
│  - 遍历组件树                                                    │
│  - 构建 JRenderContext（含悬停状态、属性值）                       │
│  - 查找并调用对应的 JControlRenderer                              │
│  - 管理 JStyleSheet（默认样式）                                   │
└──────────────────────────┬───────────────────────────────────────┘
                           │
┌──────────────────────────┼───────────────────────────────────────┐
│         JControlRenderer 体系（策略模式）                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐│
│  │ JContainer  │ │ JButton     │ │ JText       │ │ JInput      ││
│  │ Renderer    │ │ Renderer    │ │ Renderer    │ │ Renderer    ││
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘│
│         └────────────────┼──────────────┼────────────────┘       │
│                          │              │                         │
│          ┌───────────────┼──────────────┼───────────────┐        │
│          │          JCardRenderer    JCustomRenderer     │        │
│          └───────────────┴──────────────┴───────────────┘        │
│                                                                   │
│  JControlRendererRegistry（注册中心）                              │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │ JComponentType → JControlRenderer* 映射表                    │ │
│  │ register(type, renderer)  /  unregister(type)               │ │
│  │ get(type) → JControlRenderer*                               │ │
│  └─────────────────────────────────────────────────────────────┘ │
└──────────────────────────┬───────────────────────────────────────┘
                           │
┌──────────────────────────┼───────────────────────────────────────┐
│              JDirect2DRenderer（底层图形API封装）                  │
│                                                                   │
│  现有接口:          扩展接口（本文档新增）:                        │
│  - fillRect         - drawTextAligned (支持左/中/右对齐)          │
│  - drawRect         - measureText    (测量文本像素尺寸)           │
│  - fillRoundedRect  - drawEllipse    (绘制椭圆，用于RadioButton)  │
│  - drawRoundedRect  - fillEllipse    (填充椭圆)                   │
│  - drawLine                                                    │
│  - drawText                                                    │
│  - clear                                                       │
│  - beginDraw / endDraw                                         │
│  - getDpiX / getDpiY                                           │
└──────────────────────────────────────────────────────────────────┘
```

### 2.2 模块职责表

| 模块 | 文件 | 职责 |
|------|------|------|
| `JControlRenderer` | `ControlRenderer.h` | 抽象基类，定义 `render(context)` 接口 |
| `JContainerRenderer` | `ControlRenderer.h/.cpp` | 渲染 Container（灰色背景） |
| `JButtonRenderer` | `ControlRenderer.h/.cpp` | 渲染 Button（圆角矩形 + 文本 + 悬停） |
| `JTextRenderer` | `ControlRenderer.h/.cpp` | 渲染 Text（读取 FontSize/TextColor） |
| `JInputRenderer` | `ControlRenderer.h/.cpp` | 渲染 Input（白底 + 灰边框 + 文本） |
| `JCardRenderer` | `ControlRenderer.h/.cpp` | 渲染 Card（白底圆角矩形 + 灰边框 + 阴影） |
| `JStyleSheet` | `StyleSheet.h` | 统一默认样式：颜色、字体、圆角、间距等 |
| `JRenderContext` | `ControlRenderer.h` | 渲染上下文：rect + properties + hover + 指针 |
| `JControlRendererRegistry` | `ControlRenderer.h` | 注册中心：type → renderer 映射 |
| `JRendererFacade` | `ControlRenderer.h/.cpp` | 门面：遍历 + 构建上下文 + 分发渲染 |
| `JDirect2DRenderer` 扩展 | `Direct2DRenderer.h/.cpp` | 新增 drawTextAligned / measureText |

---

## 3. JDirect2DRenderer 扩展接口设计

### 3.1 新增接口分析

通过分析 `JControlRenderer` 体系需要的渲染能力，现有 `JDirect2DRenderer` 缺少以下能力：

| 缺失能力 | 当前状态 | 影响 |
|---------|---------|------|
| 文本左/右对齐 | `drawText` 内部硬编码 `DWRITE_TEXT_ALIGNMENT_CENTER` | 无法实现左对齐的输入框文本 |
| 文本尺寸测量 | 无 | 无法实现按内容自适应大小的 Label |
| 填充椭圆 | 无 | 无法渲染 RadioButton、圆形头像 |

### 3.2 新增接口定义

```cpp
// ===== 新增到 include/aether/Direct2DRenderer.h =====

/**
 * 文本对齐方式枚举
 */
enum class JTextAlignment : uint8_t {
    Left,    // 左对齐
    Center,  // 居中对齐
    Right    // 右对齐
};

class JDirect2DRenderer {
public:
    // ... 现有接口保持不变 ...

    // ===== 新增接口 =====

    /**
     * 绘制文本（支持对齐方式）
     * @param text 文本内容
     * @param rect 文本区域
     * @param color 文本颜色
     * @param fontSize 字体大小
     * @param fontName 字体名称
     * @param alignment 水平对齐方式，默认居中
     */
    void drawTextAligned(const std::string& text, const JRect& rect, const JColor& color,
                         float fontSize = 16.0f, const std::string& fontName = "Segoe UI",
                         JTextAlignment alignment = JTextAlignment::Center);

    /**
     * 测量文本的像素尺寸
     * @param text 文本内容
     * @param fontSize 字体大小
     * @param fontName 字体名称
     * @return 文本的宽度和高度（DIP单位）
     */
    JSize measureText(const std::string& text, float fontSize = 16.0f,
                      const std::string& fontName = "Segoe UI") const;

    /**
     * 绘制椭圆边框
     * @param rect 外接矩形
     * @param color 边框颜色
     * @param strokeWidth 线条宽度
     */
    void drawEllipse(const JRect& rect, const JColor& color, float strokeWidth = 1.0f);

    /**
     * 填充椭圆
     * @param rect 外接矩形
     * @param color 填充颜色
     */
    void fillEllipse(const JRect& rect, const JColor& color);

private:
    // 新增：获取带指定对齐方式的文本格式（缓存）
    IDWriteTextFormat* getTextFormat(float fontSize, const std::string& fontName,
                                     JTextAlignment alignment);
    
    // 现有缓存保持不变
    std::unordered_map<uint32_t, ID2D1SolidColorBrush*> brushCache_;
    std::unordered_map<std::string, IDWriteTextFormat*> textFormatCache_;
    // 新增：带对齐方式的文本格式缓存（key = "fontName_size_align"）
    std::unordered_map<std::string, IDWriteTextFormat*> alignedTextFormatCache_;
};
```

### 3.3 drawText 兼容处理

原有 `drawText` 内部改为调用 `drawTextAligned`：
```cpp
void JDirect2DRenderer::drawText(const std::string& text, const JRect& rect,
                                  const JColor& color, float fontSize, const std::string& fontName) {
    drawTextAligned(text, rect, color, fontSize, fontName, JTextAlignment::Center);
}
```

这样既保持向后兼容，又消除了重复代码。

### 3.4 measureText 实现要点

```cpp
JSize JDirect2DRenderer::measureText(const std::string& text, float fontSize,
                                      const std::string& fontName) const {
    // 转换文本为宽字符
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    std::wstring wText(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wText[0], len);
    
    // 获取文本格式
    auto* format = getTextFormat(fontSize, fontName, JTextAlignment::Left);
    if (!format) return JSize{0, 0};
    
    // 使用 IDWriteTextLayout 测量
    IDWriteTextLayout* layout = nullptr;
    writeFactory_->CreateTextLayout(wText.c_str(), static_cast<UINT32>(wText.length()),
                                     format, 1e6f, 1e6f, &layout);
    if (!layout) return JSize{0, 0};
    
    DWRITE_TEXT_METRICS metrics;
    layout->GetMetrics(&metrics);
    layout->Release();
    
    return JSize{metrics.width, metrics.height};
}
```

---

## 4. JControlRenderer 基类与子类设计

### 4.1 基类 — JControlRenderer

```cpp
// 文件: include/aether/ControlRenderer.h

namespace jaether {

// 前向声明
class JDirect2DRenderer;
class JStyleSheet;
struct JRenderContext;

/**
 * 控件渲染器抽象基类
 * 
 * 策略模式的核心抽象。每个具体控件类型对应一个子类实现。
 * 用户可通过继承此类并注册到Registry来定制任意控件的渲染行为。
 */
class JControlRenderer {
public:
    virtual ~JControlRenderer() = default;

    /**
     * 渲染单个控件
     * @param renderer  底层 Direct2D 渲染器
     * @param ctx       渲染上下文（位置、属性、状态）
     * @param style     当前生效的样式表
     */
    virtual void render(JDirect2DRenderer* renderer,
                        const JRenderContext& ctx,
                        const JStyleSheet& style) = 0;

    /**
     * 获取此渲染器支持的控件类型
     * @return 控件类型标识
     */
    virtual JComponentType getComponentType() const = 0;
};

```

### 4.2 JRenderContext — 渲染上下文

```cpp
/**
 * 渲染上下文结构体
 * 
 * 封装一次渲染调用所需的全部输入信息
 * 由 JRendererFacade 在遍历组件树时构建
 */
struct JRenderContext {
    JComponentHandle handle;           // 组件句柄
    JComponentType type;              // 组件类型
    JRect absoluteRect;               // 绝对窗口坐标的渲染矩形
    const JPropertyBlock* properties; // 组件属性块指针
    bool isHovered = false;           // 鼠标是否悬停
    bool isFocused = false;           // 是否拥有焦点
    float mouseX = -1.0f;             // 鼠标X（-1表示无效）
    float mouseY = -1.0f;             // 鼠标Y（-1表示无效）
};
```

### 4.3 JContainerRenderer — 容器渲染器

```cpp
/**
 * 容器控件渲染器
 * 
 * 渲染逻辑：填充柔和灰色背景
 * 注意：Container 不拦截鼠标事件（命中测试时被排除），因此不响应悬停
 */
class JContainerRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override {
        renderer->fillRect(ctx.absoluteRect, style.containerBackground);
    }
    
    JComponentType getComponentType() const override {
        return JComponentType::Container;
    }
};
```

### 4.4 JButtonRenderer — 按钮渲染器

```cpp
/**
 * 按钮控件渲染器
 * 
 * 渲染逻辑：
 * 1. 绘制圆角矩形背景（普通/悬停 不同颜色）
 * 2. 绘制 Button 子 Text 组件的文本居中
 * 3. 从属性读取 variant（primary/danger 等）决定配色
 */
class JButtonRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override {
        // 步骤1：确定按钮背景色（优先用属性variant，其次用悬停状态）
        JColor bgColor = style.buttonBackground;
        
        // 从属性读取 variant 字段
        auto* bgProp = ctx.properties->getProperty(JPropertyId::BackgroundColor);
        if (bgProp && bgProp->is<std::string>()) {
            const std::string& variant = bgProp->get<std::string>();
            if (variant == "primary") bgColor = style.buttonPrimaryBackground;
            else if (variant == "danger") bgColor = JColor(0.85f, 0.2f, 0.2f, 1.0f);
        }
        
        // 悬停状态覆盖
        if (ctx.isHovered) {
            bgColor = style.buttonHoverBackground;
        }
        
        // 步骤2：绘制圆角矩形背景
        renderer->fillRoundedRect(ctx.absoluteRect, style.buttonRadius,
                                  style.buttonRadius, bgColor);
        
        // 步骤3：绘制按钮文本
        auto* textProp = ctx.properties->getProperty(JPropertyId::Text);
        if (textProp && textProp->is<std::string>()) {
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

    JComponentType getComponentType() const override {
        return JComponentType::Button;
    }
};
```

### 4.5 JTextRenderer — 文本渲染器

```cpp
/**
 * 文本控件渲染器
 * 
 * 渲染逻辑：
 * 1. 从属性读取 FontSize、TextColor（有则用属性值，无则用样式默认值）
 * 2. 左对齐绘制文本（非居中，方便阅读型文本）
 */
class JTextRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override {
        auto* textProp = ctx.properties->getProperty(JPropertyId::Text);
        if (!textProp || !textProp->is<std::string>()) return;
        
        // 字号：优先从属性读取
        float fontSize = style.textFontSize;
        auto* fsProp = ctx.properties->getProperty(JPropertyId::FontSize);
        if (fsProp) {
            if (fsProp->is<float>()) fontSize = fsProp->get<float>();
            else if (fsProp->is<int>()) fontSize = static_cast<float>(fsProp->get<int>());
        }
        
        // 颜色：优先从属性读取
        JColor textColor = style.textColor;
        auto* tcProp = ctx.properties->getProperty(JPropertyId::TextColor);
        // TextColor 当前以字符串存储，后续可扩展为 JColor 类型
        // 此处使用默认颜色
        
        // 对齐方式
        JTextAlignment align = JTextAlignment::Left;
        auto* taProp = ctx.properties->getProperty(JPropertyId::TextAlign);
        // TextAlign 当前未完全支持，默认左对齐
        (void)taProp;
        
        renderer->drawTextAligned(textProp->get<std::string>(), ctx.absoluteRect,
                                  textColor, fontSize, style.defaultFontName, align);
    }

    JComponentType getComponentType() const override {
        return JComponentType::Text;
    }
};
```

### 4.6 JInputRenderer — 输入框渲染器

```cpp
/**
 * 输入框控件渲染器
 * 
 * 渲染逻辑：
 * 1. 填充白色背景
 * 2. 绘制灰色边框
 * 3. 左对齐绘制输入文本
 */
class JInputRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override {
        // 白色背景
        renderer->fillRect(ctx.absoluteRect, style.inputBackground);
        // 灰色边框
        renderer->drawRect(ctx.absoluteRect, style.inputBorderColor, style.inputBorderWidth);
        
        // 左对齐绘制文本
        auto* textProp = ctx.properties->getProperty(JPropertyId::Text);
        if (textProp && textProp->is<std::string>()) {
            // 内边距：留 6px 左边距
            JRect textRect = ctx.absoluteRect;
            textRect.x += style.inputTextPadding;
            textRect.width -= style.inputTextPadding * 2;
            
            float fontSize = style.inputFontSize;
            auto* fsProp = ctx.properties->getProperty(JPropertyId::FontSize);
            if (fsProp) {
                if (fsProp->is<float>()) fontSize = fsProp->get<float>();
                else if (fsProp->is<int>()) fontSize = static_cast<float>(fsProp->get<int>());
            }
            
            renderer->drawTextAligned(textProp->get<std::string>(), textRect,
                                      style.inputTextColor, fontSize,
                                      style.defaultFontName, JTextAlignment::Left);
        }
    }

    JComponentType getComponentType() const override {
        return JComponentType::Input;
    }
};
```

### 4.7 JCardRenderer — 卡片渲染器

```cpp
/**
 * 卡片容器渲染器
 * 
 * 渲染逻辑：
 * 1. 填充白色圆角矩形背景（上方较深阴影）
 * 2. 绘制灰色圆角矩形边框
 */
class JCardRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override {
        // 阴影效果：在卡片下方偏移处绘制一个半透明的深色矩形
        JRect shadowRect = ctx.absoluteRect;
        shadowRect.x += style.cardShadowOffset;
        shadowRect.y += style.cardShadowOffset;
        renderer->fillRoundedRect(shadowRect, style.cardRadius, style.cardRadius,
                                  style.cardShadowColor);
        
        // 白色背景
        renderer->fillRoundedRect(ctx.absoluteRect, style.cardRadius, style.cardRadius,
                                  style.cardBackground);
        // 灰色边框
        renderer->drawRoundedRect(ctx.absoluteRect, style.cardRadius, style.cardRadius,
                                  style.cardBorderColor, style.cardBorderWidth);
    }

    JComponentType getComponentType() const override {
        return JComponentType::Card;
    }
};
```

---

## 5. JStyleSheet — 统一默认样式系统

### 5.1 设计原则

- 所有颜色、字号、圆角等视觉参数集中定义，一键切换主题
- 提供合理的默认值（基于现代扁平化设计）
- 每个 Renderer 渲染时从 JStyleSheet 读取参数，不硬编码

### 5.2 类定义

```cpp
// 文件: include/aether/StyleSheet.h

namespace jaether {

/**
 * 统一默认样式表
 * 
 * 集中管理所有控件的视觉参数。
 * 用户可通过修改 JStyleSheet 实例来实现自定义主题。
 */
struct JStyleSheet {
    // ===== 全局默认 =====
    std::string defaultFontName = "Microsoft YaHei";  // 默认中文字体
    JColor clearColor = JColor(0.96f, 0.96f, 0.96f, 1.0f);  // 画布底色
    
    // ===== Container =====
    JColor containerBackground = JColor(0.95f, 0.95f, 0.95f, 1.0f);
    
    // ===== Button =====
    JColor  buttonBackground       = JColor(0.2f, 0.5f, 0.9f, 1.0f);   // 默认蓝
    JColor  buttonPrimaryBackground = JColor(0.15f, 0.45f, 0.85f, 1.0f); // primary 深蓝
    JColor  buttonHoverBackground   = JColor(1.0f, 0.85f, 0.0f, 1.0f);  // 悬停黄
    JColor  buttonTextColor         = JColor(1.0f, 1.0f, 1.0f, 1.0f);   // 白字
    float   buttonFontSize          = 14.0f;
    float   buttonRadius            = 5.0f;
    
    // ===== Text =====
    JColor  textColor      = JColor(0.1f, 0.1f, 0.1f, 1.0f);   // 深灰黑
    float   textFontSize   = 14.0f;
    
    // ===== Input =====
    JColor  inputBackground   = JColor(1.0f, 1.0f, 1.0f, 1.0f);    // 白
    JColor  inputBorderColor  = JColor(0.6f, 0.6f, 0.6f, 1.0f);    // 灰边框
    JColor  inputTextColor    = JColor(0.1f, 0.1f, 0.1f, 1.0f);    // 深灰黑
    float   inputFontSize     = 14.0f;
    float   inputBorderWidth  = 1.0f;
    float   inputTextPadding  = 6.0f;  // 输入框文字左边距
    
    // ===== Card =====
    JColor  cardBackground    = JColor(1.0f, 1.0f, 1.0f, 1.0f);      // 白
    JColor  cardBorderColor   = JColor(0.85f, 0.85f, 0.85f, 1.0f);   // 浅灰边框
    JColor  cardShadowColor   = JColor(0.0f, 0.0f, 0.0f, 0.08f);     // 半透明阴影
    float   cardRadius        = 8.0f;
    float   cardBorderWidth   = 1.0f;
    float   cardShadowOffset  = 3.0f;  // 阴影偏移
    
    // ===== CheckBox (预留) =====
    float   checkboxSize      = 18.0f;
    JColor  checkboxCheckedBg = JColor(0.2f, 0.5f, 0.9f, 1.0f);
    JColor  checkboxBorderColor = JColor(0.5f, 0.5f, 0.5f, 1.0f);
    
    // ===== ScrollView (预留) =====
    JColor  scrollbarColor    = JColor(0.7f, 0.7f, 0.7f, 0.5f);
    float   scrollbarWidth    = 8.0f;
    
    // ===== Slider (预留) =====
    JColor  sliderTrackColor  = JColor(0.85f, 0.85f, 0.85f, 1.0f);
    JColor  sliderThumbColor  = JColor(0.2f, 0.5f, 0.9f, 1.0f);
    float   sliderTrackHeight = 4.0f;
    float   sliderThumbRadius = 10.0f;

    /**
     * 创建浅色主题（默认）
     */
    static JStyleSheet light();

    /**
     * 创建深色主题
     */
    static JStyleSheet dark();
};
```

### 5.3 深色主题工厂方法

```cpp
JStyleSheet JStyleSheet::dark() {
    JStyleSheet s;
    s.clearColor            = JColor(0.12f, 0.12f, 0.14f, 1.0f);
    s.containerBackground   = JColor(0.18f, 0.18f, 0.20f, 1.0f);
    s.buttonBackground      = JColor(0.25f, 0.50f, 0.85f, 1.0f);
    s.buttonTextColor       = JColor(0.95f, 0.95f, 0.95f, 1.0f);
    s.textColor             = JColor(0.90f, 0.90f, 0.90f, 1.0f);
    s.inputBackground       = JColor(0.22f, 0.22f, 0.24f, 1.0f);
    s.inputBorderColor      = JColor(0.40f, 0.40f, 0.45f, 1.0f);
    s.inputTextColor        = JColor(0.90f, 0.90f, 0.90f, 1.0f);
    s.cardBackground        = JColor(0.22f, 0.22f, 0.24f, 1.0f);
    s.cardBorderColor       = JColor(0.35f, 0.35f, 0.40f, 1.0f);
    return s;
}
```

---

## 6. JControlRendererRegistry — 渲染器注册中心

### 6.1 类定义

```cpp
/**
 * 控件渲染器注册中心
 * 
 * 维护 JComponentType → JControlRenderer* 的映射表
 * 支持注册、注销、遍历
 */
class JControlRendererRegistry {
public:
    JControlRendererRegistry();
    ~JControlRendererRegistry();

    /**
     * 注册/替换指定控件类型的渲染器
     * @param type     控件类型
     * @param renderer 渲染器实例（Registry 不接管所有权，调用方管理生命周期）
     */
    void registerRenderer(JComponentType type, JControlRenderer* renderer);

    /**
     * 注销指定控件类型的渲染器
     * @param type 控件类型
     */
    void unregisterRenderer(JComponentType type);

    /**
     * 获取指定控件类型的渲染器
     * @param type 控件类型
     * @return 渲染器指针，未注册返回 nullptr
     */
    JControlRenderer* getRenderer(JComponentType type) const;

    /**
     * 检查是否有注册指定类型的渲染器
     */
    bool hasRenderer(JComponentType type) const;

    /**
     * 获取所有已注册的控件类型
     */
    std::vector<JComponentType> getRegisteredTypes() const;

    /**
     * 初始化所有内置渲染器（Container/Button/Text/Input/Card）
     */
    static void registerDefaults(JControlRendererRegistry& registry);

private:
    std::unordered_map<JComponentType, JControlRenderer*> map_;
};
```

### 6.2 内置渲染器注册

```cpp
void JControlRendererRegistry::registerDefaults(JControlRendererRegistry& registry) {
    // 这些是静态实例，生命周期与进程一致
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
```

---

## 7. JRendererFacade — 对外的统一渲染门面

### 7.1 类定义

```cpp
/**
 * 统一渲染门面
 * 
 * 应用层通过此门面完成所有渲染工作，无需关心内部 Renderer 细节。
 * 负责：遍历组件树 → 构建 JRenderContext → 查找 Renderer → 调用 render()
 */
class JRendererFacade {
public:
    /**
     * 构造函数
     * @param renderer 底层 Direct2D 渲染器
     * @param storage  组件存储（用于遍历和命中测试）
     */
    JRendererFacade(JDirect2DRenderer* renderer, JComponentStorage& storage);

    /**
     * 渲染所有组件
     * @param mouseX 当前鼠标X坐标（-1表示不检测悬停）
     * @param mouseY 当前鼠标Y坐标
     */
    void renderAll(float mouseX = -1.0f, float mouseY = -1.0f);

    /**
     * 渲染单个组件
     * @param handle 组件句柄
     * @param mouseX 鼠标X（-1=不检测悬停）
     * @param mouseY 鼠标Y
     */
    void renderOne(JComponentHandle handle, float mouseX = -1.0f, float mouseY = -1.0f);

    /**
     * 获取渲染器注册中心（用于注册自定义渲染器）
     */
    JControlRendererRegistry& getRegistry() { return registry_; }

    /**
     * 获取当前样式表（可读写，用于动态修改样式）
     */
    JStyleSheet& getStyleSheet() { return styleSheet_; }
    const JStyleSheet& getStyleSheet() const { return styleSheet_; }

    /**
     * 设置样式表（替换整个主题）
     */
    void setStyleSheet(const JStyleSheet& sheet) { styleSheet_ = sheet; }

    /**
     * 清空画布
     */
    void clearCanvas();

private:
    /**
     * 构建单个组件的渲染上下文
     */
    JRenderContext buildContext(JComponentHandle handle, float mouseX, float mouseY);

    JDirect2DRenderer* renderer_;
    JComponentStorage& storage_;
    JControlRendererRegistry registry_;
    JStyleSheet styleSheet_;

    // 循环引用检测（防止无限递归）
    std::unordered_set<int32_t> visitedIndices_;
};
```

### 7.2 核心渲染流程

```cpp
void JRendererFacade::renderAll(float mouseX, float mouseY) {
    visitedIndices_.clear();
    
    // 步骤1：遍历所有活跃组件
    storage_.forEach([this, mouseX, mouseY](JComponentHandle handle) {
        auto* entry = storage_.getComponent(handle);
        if (!entry || !entry->visible) return;
        
        // 步骤2：构建渲染上下文（含绝对坐标、悬停状态）
        JRenderContext ctx = buildContext(handle, mouseX, mouseY);
        
        // 步骤3：查找对应的渲染器
        JControlRenderer* ctrlRenderer = registry_.getRenderer(entry->type);
        if (!ctrlRenderer) return;  // 未注册渲染器的控件跳过
        
        // 步骤4：调用渲染
        ctrlRenderer->render(renderer_, ctx, styleSheet_);
    });
}

JRenderContext JRendererFacade::buildContext(JComponentHandle handle,
                                              float mouseX, float mouseY) {
    JRenderContext ctx;
    ctx.handle = handle;
    
    auto* entry = storage_.getComponent(handle);
    if (!entry) return ctx;
    
    ctx.type = entry->type;
    ctx.absoluteRect = storage_.getAbsoluteBounds(handle);
    ctx.properties = &entry->properties;
    ctx.isHovered = storage_.containsPoint(handle, mouseX, mouseY);
    ctx.mouseX = mouseX;
    ctx.mouseY = mouseY;
    
    return ctx;
}
```

---

## 8. 替换现有分散代码的方案

### 8.1 AetherApplication 修改

```cpp
// ===== 修改前 (src/AetherApplication.cpp) =====
void JAetherApplication::render() {
    if (!renderer_ || !logicLayer_) return;
    renderer_->beginDraw();
    renderer_->clear(JColor(0.95f, 0.95f, 0.95f, 1.0f));
    
    auto& storage = logicLayer_->getStorage();
    storage.forEach([this, &storage](JComponentHandle handle) {
        // ... 45行的 switch/case ...
    });
    
    renderer_->endDraw();
}

// ===== 修改后 =====
#include "aether/ControlRenderer.h"

void JAetherApplication::render() {
    if (!renderer_ || !logicLayer_) return;
    
    // 惰性初始化渲染门面
    if (!renderFacade_) {
        renderFacade_ = std::make_unique<JRendererFacade>(
            renderer_.get(), logicLayer_->getStorage());
    }
    
    renderer_->beginDraw();
    renderFacade_->clearCanvas();
    renderFacade_->renderAll(lastMouseX_, lastMouseY_);
    renderer_->endDraw();
}
```

### 8.2 TodoAppWindow 修改

```cpp
// ===== 修改前 (todo-app/main.cpp render() 方法) =====
void render() {
    // ... 95行的 switch/case + 悬停检测 + 属性读取 ...
}

// ===== 修改后 =====
void render() {
    if (!renderer_ || !logicLayer_) return;
    
    if (!renderFacade_) {
        renderFacade_ = std::make_unique<jaether::JRendererFacade>(
            renderer_.get(), logicLayer_->getStorage());
    }
    
    PAINTSTRUCT ps;
    BeginPaint(hwnd_, &ps);

    renderer_->beginDraw();
    renderFacade_->clearCanvas();
    renderFacade_->renderAll(mouseX, mouseY);
    renderer_->endDraw();

    EndPaint(hwnd_, &ps);
}

// 在 TodoAppWindow 类中新增成员
std::unique_ptr<jaether::JRendererFacade> renderFacade_;
float mouseX = -1.0f;
float mouseY = -1.0f;
```

### 8.3 替换前后对比

| 方面 | 替换前 | 替换后 |
|------|--------|--------|
| AetherApplication 渲染代码 | ~45行 switch/case | ~8行 门面调用 |
| TodoAppWindow 渲染代码 | ~95行 switch/case | ~10行 门面调用 |
| Button 悬停颜色 | 硬编码 rgb(1,1,0) | JStyleSheet::buttonHoverBackground |
| 字体大小 | 硬编码 16.0f | 从 JPropertyId::FontSize 读取 |
| 新增控件 | 改两处 switch/case | 注册一个 Renderer |
| 换主题 | 改多处硬编码 | JStyleSheet::dark() 一键切换 |
| Card 支持 | 仅 todo-app | AetherApplication 自动支持 |

---

## 9. 扩展性设计：用户自定义渲染器

### 9.1 自定义渲染器示例

用户创建一个新控件类型 `ProgressBar`：

```cpp
// 1. 在 types.h 中新增枚举值（或在 Custom 基础上扩展）
// JComponentType::Custom + 约定

// 2. 实现自定义渲染器
class JProgressBarRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override {
        // 读取进度值（存储在 UserDefined 属性中）
        auto* progressProp = ctx.properties->getProperty(JPropertyId::UserDefined + 10);
        float progress = 0.5f;
        if (progressProp && progressProp->is<float>())
            progress = progressProp->get<float>();
        
        // 背景轨道
        renderer->fillRoundedRect(ctx.absoluteRect, 4.0f, 4.0f,
                                  JColor(0.85f, 0.85f, 0.85f, 1.0f));
        
        // 填充进度
        JRect fillRect = ctx.absoluteRect;
        fillRect.width *= progress;
        renderer->fillRoundedRect(fillRect, 4.0f, 4.0f,
                                  style.buttonPrimaryBackground);
    }

    JComponentType getComponentType() const override {
        return static_cast<JComponentType>(JComponentType::Custom); // 映射到自定义类型
    }
};

// 3. 注册到门面
renderFacade_->getRegistry().registerRenderer(
    static_cast<JComponentType>(JComponentType::Custom),
    &progressBarRenderer);
```

### 9.2 扩展点总结

| 扩展场景 | 修改方式 | 影响范围 |
|---------|---------|---------|
| 新增控件类型渲染 | 实现新的 JControlRenderer 子类 + 注册 | 仅新增文件 |
| 替换已有控件渲染 | 调用 registerRenderer 覆盖 | 1行代码 |
| 修改默认颜色/字体 | 修改 JStyleSheet 字段值 | 1-5行代码 |
| 一键切换深色主题 | `facade.setStyleSheet(JStyleSheet::dark())` | 1行代码 |
| 新增 JDirect2DRenderer 能力 | 在 Direct2DRenderer 中新增方法 | 新增接口+实现 |

---

## 10. 新增文件与修改文件清单

### 10.1 新增文件

| 文件 | 说明 |
|------|------|
| `include/aether/ControlRenderer.h` | JControlRenderer 基类、各子类声明、JRenderContext、JControlRendererRegistry、JRendererFacade |
| `include/aether/StyleSheet.h` | JStyleSheet 统一样式表 |
| `src/ControlRenderer.cpp` | 各渲染器实现、JRendererFacade 实现、JControlRendererRegistry 实现 |
| `src/StyleSheet.cpp` | JStyleSheet::light() / JStyleSheet::dark() 工厂方法 |

### 10.2 修改文件

| 文件 | 修改内容 |
|------|----------|
| `include/aether/Direct2DRenderer.h` | 新增 JTextAlignment 枚举、drawTextAligned、measureText、drawEllipse、fillEllipse、alignedTextFormatCache_ |
| `src/Direct2DRenderer.cpp` | 实现上述新增方法，原有 drawText 内部委托给 drawTextAligned |
| `src/AetherApplication.cpp` | render() 方法用 JRendererFacade 替换 switch/case（删除 ~45行） |
| `todo-app/main.cpp` | render() 方法用 JRendererFacade 替换 switch/case（删除 ~95行） |

---

## 11. 实现计划

### 阶段一：JDirect2DRenderer 扩展（0.5天）

| 任务 | 说明 |
|------|------|
| 新增 `JTextAlignment` 枚举 | `include/aether/Direct2DRenderer.h` |
| 实现 `drawTextAligned` | 基于现有 `drawText`，增加 alignment 参数和缓存 |
| 实现 `measureText` | 使用 `IDWriteTextLayout` 测量 |
| 实现 `drawEllipse` / `fillEllipse` | 使用 `D2D1_ELLIPSE` + `DrawEllipse` / `FillEllipse` |
| 重构 `drawText` | 内部委托给 `drawTextAligned(..., Center)` |
| 单元测试 | 验证新旧接口行为一致 |

### 阶段二：控件渲染器体系实现（1天）

| 任务 | 说明 |
|------|------|
| `JControlRenderer` 抽象基类 | 头文件 |
| `JRenderContext` | 头文件 |
| `JStyleSheet` | 头文件 + 实现（light/dark 工厂方法） |
| `JContainerRenderer` | 头文件 + 实现 |
| `JButtonRenderer` | 头文件 + 实现（含 variant/hover 处理） |
| `JTextRenderer` | 头文件 + 实现（含 FontSize 属性读取） |
| `JInputRenderer` | 头文件 + 实现 |
| `JCardRenderer` | 头文件 + 实现（含阴影） |
| `JControlRendererRegistry` | 头文件 + 实现（registerDefaults） |
| `JRendererFacade` | 头文件 + 实现（renderAll/renderOne/buildContext） |
| 单元测试 | 各 Renderer + Registry + Facade |

### 阶段三：替换现有代码 + 验证（0.5天）

| 任务 | 说明 |
|------|------|
| 替换 `AetherApplication::render()` | 删除 switch/case，接入 JRendererFacade |
| 替换 `TodoAppWindow::render()` | 删除 switch/case，接入 JRendererFacade |
| 编译运行 | aether_app.exe + TodoApp.exe 视觉效果验证 |
| 全部测试通过 | 296 测试回归 |
| 更新框架参考文档 | §18 新增 ControlRenderer 章节 |

---

## 附录 A：JStyleSheet 默认值速查表

| 参数 | 浅色主题值 | 深色主题值 |
|------|-----------|-----------|
| `clearColor` | rgb(245,245,245) | rgb(31,31,36) |
| `containerBackground` | rgb(242,242,242) | rgb(46,46,51) |
| `buttonBackground` | rgb(51,128,230) | rgb(64,128,217) |
| `buttonHoverBackground` | rgb(255,217,0) | rgb(255,217,0) |
| `buttonTextColor` | rgb(255,255,255) | rgb(242,242,242) |
| `textColor` | rgb(26,26,26) | rgb(230,230,230) |
| `inputBackground` | rgb(255,255,255) | rgb(56,56,61) |
| `cardBackground` | rgb(255,255,255) | rgb(56,56,61) |
| `buttonRadius` | 5.0f | 5.0f |
| `cardRadius` | 8.0f | 8.0f |

---

## 附录 B：变更记录

| 版本 | 日期 | 变更内容 |
|------|------|----------|
| 1.0.0 | 2026-05-25 | 初始设计 |

---

**文档结束**
