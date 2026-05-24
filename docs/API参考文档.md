# Aether UI 引擎 API 参考文档

## 📑 目录

1. [核心类](#核心类)
2. [组件系统API](#组件系统api)
3. [布局引擎API](#布局引擎api)
4. [事件系统API](#事件系统api)
5. [渲染系统API](#渲染系统api)
6. [状态管理API](#状态管理api)
7. [工具类和枚举](#工具类和枚举)

---

## 核心类

### AetherApplication

主应用类，负责应用的初始化和运行。

**头文件**: `include/aether/AetherApplication.h`

```cpp
namespace aether {

class AetherApplication {
public:
    // 创建应用实例（单例模式）
    static std::unique_ptr<AetherApplication> create();
    
    // 运行应用
    int run();
    
    // 退出应用
    void quit();
    
    // 获取主窗口句柄
    HWND getMainWindow() const;
    
    // 设置主窗口句柄
    void setMainWindow(HWND hwnd);
    
    // 获取组件存储
    ComponentStorage& getStorage();
    
    // 获取布局引擎
    LayoutEngine& getLayoutEngine();
    
    // 获取事件分发器
    EventDispatcher& getEventDispatcher();
};

}
```

**使用示例**:

```cpp
int main() {
    auto app = AetherApplication::create();
    
    if (!app) {
        return 1;
    }
    
    return app->run();
}
```

---

## 组件系统API

### ComponentStorage

组件存储管理器，负责创建、销毁和访问组件。

**头文件**: `include/aether/ComponentStorage.h`

```cpp
namespace aether {

class ComponentStorage {
public:
    // 构造函数
    ComponentStorage();
    
    // 析构函数
    ~ComponentStorage();
    
    // 创建组件
    // 参数:
    //   type - 组件类型
    //   parent - 父组件句柄，INVALID_COMPONENT_HANDLE表示无父组件
    // 返回: 新创建的组件句柄
    ComponentHandle createComponent(ComponentType type, 
                                  ComponentHandle parent = INVALID_COMPONENT_HANDLE);
    
    // 销毁组件
    // 参数:
    //   handle - 要销毁的组件句柄
    void destroyComponent(ComponentHandle handle);
    
    // 获取组件
    // 参数:
    //   handle - 组件句柄
    // 返回: 组件指针，如果句柄无效则返回nullptr
    ComponentEntry* getComponent(ComponentHandle handle);
    
    // 通过索引获取组件
    // 参数:
    //   index - 组件索引
    // 返回: 组件指针
    ComponentEntry* getComponentByIndex(int32_t index);
    
    // 获取组件数量
    size_t size() const;
    
    // 获取根组件
    ComponentHandle getRoot() const;
    
    // 获取父组件
    ComponentHandle getParent(ComponentHandle child) const;
    
    // 获取组件句柄
    // 参数:
    //   index - 组件索引
    // 返回: 组件句柄，如果索引无效则返回INVALID_COMPONENT_HANDLE
    ComponentHandle getHandle(int32_t index) const;
    
    // 验证句柄是否有效
    bool isValid(ComponentHandle handle) const;
    
    // 清空所有组件
    void clear();
    
    // 获取根组件条目
    ComponentEntry* getRootEntry();
    
    // 通过索引获取组件（const版本）
    const ComponentEntry* getComponentByIndex(int32_t index) const;
};

}
```

**关键方法详解**:

#### createComponent

```cpp
// 示例1: 创建无父组件的容器
auto container = storage->createComponent(ComponentType::Container);

// 示例2: 创建有父组件的按钮
auto button = storage->createComponent(ComponentType::Button, container);

// 示例3: 创建文本输入框
auto textField = storage->createComponent(ComponentType::TextField, container);
```

#### getComponent

```cpp
// 安全访问组件
auto handle = someHandle;
if (storage->isValid(handle)) {
    auto* entry = storage->getComponent(handle);
    if (entry) {
        // 安全使用entry
        entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    }
}
```

#### 遍历所有组件

```cpp
for (int32_t i = 0; i < static_cast<int32_t>(storage->size()); ++i) {
    auto* entry = storage->getComponentByIndex(i);
    if (entry && entry->id != INVALID_COMPONENT_ID) {
        // 处理组件
    }
}
```

---

### ComponentEntry

组件条目结构，存储组件的所有信息。

**头文件**: `include/aether/ComponentStorage.h`

```cpp
namespace aether {

struct ComponentEntry {
    // 组件唯一标识符
    int32_t id;
    
    // 组件类型
    ComponentType type;
    
    // 组件属性集
    PropertySet properties;
    
    // 布局结果（位置和尺寸）
    LayoutRect layoutResult;
    
    // 父组件索引，-1表示无父组件
    int32_t parentIndex;
    
    // 子组件索引列表
    std::vector<int32_t> childrenIndices;
    
    // 是否可见
    bool visible;
    
    // 是否启用
    bool enabled;
    
    // 是否获得焦点
    bool focused;
};

struct LayoutRect {
    float x;      // 左上角X坐标
    float y;      // 左上角Y坐标
    float width;  // 宽度
    float height; // 高度
    
    // 构造函数
    LayoutRect() : x(0), y(0), width(0), height(0) {}
    LayoutRect(float x, float y, float width, float height) 
        : x(x), y(y), width(width), height(height) {}
    
    // 检查是否有效
    bool isValid() const {
        return width > 0 && height > 0;
    }
    
    // 检查点是否在矩形内
    bool contains(float px, float py) const {
        return px >= x && px <= x + width && 
               py >= y && py <= y + height;
    }
    
    // 获取右边界
    float right() const { return x + width; }
    
    // 获取下边界
    float bottom() const { return y + height; }
};

}
```

**使用示例**:

```cpp
auto button = storage->createComponent(ComponentType::Button, root);
auto* entry = storage->getComponent(button);

// 设置布局
entry->layoutResult = {100, 50, 120, 40};

// 设置属性
entry->properties.setProperty(PropertyId::Text, PropertyValue("Click"));

// 设置状态
entry->visible = true;
entry->enabled = true;

// 访问布局结果
float buttonRight = entry->layoutResult.x + entry->layoutResult.width;
```

---

### ComponentHandle

组件句柄，用于安全引用组件。

**头文件**: `include/aether/ComponentStorage.h`

```cpp
namespace aether {

// 组件句柄结构
struct ComponentHandle {
    int32_t index;      // 组件索引
    int32_t generation;  // 组件代数
    
    // 默认构造函数
    ComponentHandle() : index(-1), generation(0) {}
    
    // 构造函数
    ComponentHandle(int32_t idx, int32_t gen) 
        : index(idx), generation(gen) {}
    
    // 检查句柄是否有效
    bool isValid() const {
        return index >= 0;
    }
    
    // 相等比较
    bool operator==(const ComponentHandle& other) const {
        return index == other.index && generation == other.generation;
    }
    
    bool operator!=(const ComponentHandle& other) const {
        return !(*this == other);
    }
};

// 无效的组件句柄常量
const ComponentHandle INVALID_COMPONENT_HANDLE(-1, 0);

}
```

**使用示例**:

```cpp
// 创建组件并保存句柄
auto button = storage->createComponent(ComponentType::Button, parent);

// 检查有效性
if (button.isValid()) {
    // 安全使用
    auto* entry = storage->getComponent(button);
}

// 保存句柄供后续使用
ComponentHandle savedButton = button;

// 延迟访问
void someFunction(ComponentHandle h) {
    if (h.isValid()) {
        auto* entry = storage->getComponent(h);
        // ...
    }
}
```

---

### ComponentType

组件类型枚举。

**头文件**: `include/aether/ComponentStorage.h`

```cpp
namespace aether {

// 组件类型
enum class ComponentType {
    Container = 0,   // 容器组件，用于布局
    Button = 1,     // 按钮组件
    TextField = 2,   // 文本输入框
    Label = 3,       // 标签组件
    Checkbox = 4     // 复选框
};

}
```

---

## 属性系统API

### PropertyId

属性ID枚举。

**头文件**: `include/aether/PropertySet.h`

```cpp
namespace aether {

// 属性ID
enum class PropertyId {
    // 尺寸属性
    Width,           // 宽度
    Height,          // 高度
    
    // Flexbox属性
    FlexDirection,   // 主轴方向
    FlexWrap,        // 换行方式
    FlexFlow,        // 流动方向
    JustifyContent,  // 主轴对齐方式
    AlignItems,      // 交叉轴对齐方式
    AlignContent,    // 多行对齐方式
    FlexGrow,       // 放大因子
    FlexShrink,     // 收缩因子
    FlexBasis,      // 基准尺寸
    
    // 外边距
    MarginLeft,
    MarginTop,
    MarginRight,
    MarginBottom,
    
    // 内边距
    PaddingLeft,
    PaddingTop,
    PaddingRight,
    PaddingBottom,
    
    // 样式属性
    BackgroundColor, // 背景颜色
    TextColor,      // 文本颜色
    BorderColor,    // 边框颜色
    BorderWidth,    // 边框宽度
    
    // 文本属性
    Text,            // 文本内容
    PlaceholderText, // 占位文本
    FontSize,        // 字体大小
    FontFamily,      // 字体家族
    
    // 交互属性
    OnClick,        // 点击回调
    OnHover,        // 悬停回调
    OnFocus,        // 聚焦回调
    
    // 布局属性
    PositionX,      // X坐标
    PositionY,      // Y坐标
    
    // 特殊属性
    ZIndex,         // Z轴顺序
    Opacity,        // 不透明度
};

}
```

---

### PropertyValue

属性值类型，使用变体存储支持多种类型。

**头文件**: `include/aether/PropertySet.h`

```cpp
namespace aether {

class PropertyValue {
public:
    // 默认构造函数
    PropertyValue();
    
    // 整数构造函数
    PropertyValue(int value);
    
    // 浮点数构造函数
    PropertyValue(float value);
    
    // 双精度浮点数构造函数
    PropertyValue(double value);
    
    // 字符串构造函数
    PropertyValue(const std::string& value);
    
    // 宽字符串构造函数
    PropertyValue(const std::wstring& value);
    
    // 布尔构造函数
    PropertyValue(bool value);
    
    // 枚举构造函数
    template<typename T>
    PropertyValue(T enumValue) 
        : value_(static_cast<int>(enumValue)) {}
    
    // 复制构造函数
    PropertyValue(const PropertyValue& other);
    
    // 移动构造函数
    PropertyValue(PropertyValue&& other) noexcept;
    
    // 赋值运算符
    PropertyValue& operator=(const PropertyValue& other);
    
    // 类型检查
    template<typename T>
    bool is() const;
    
    // 类型转换
    template<typename T>
    T get() const;
    
    // 获取类型ID
    int getTypeId() const;
    
    // 获取字符串表示
    std::string toString() const;
    
private:
    int typeId_;                    // 类型ID
    std::variant<
        int,
        float,
        std::string,
        std::wstring,
        bool
    > value_;                      // 值存储
};

}
```

**使用示例**:

```cpp
// 整数
PropertyValue intVal(100);
if (intVal.is<int>()) {
    int v = intVal.get<int>();
}

// 浮点数
PropertyValue floatVal(50.5f);
if (floatVal.is<float>()) {
    float v = floatVal.get<float>();
}

// 字符串
PropertyValue strVal("Hello");
if (strVal.is<std::string>()) {
    std::string v = strVal.get<std::string>();
}

// 枚举
PropertyValue enumVal(FlexDirection::Row);
if (enumVal.is<FlexDirection>()) {
    FlexDirection v = enumVal.get<FlexDirection>();
}

// 布尔
PropertyValue boolVal(true);
if (boolVal.is<bool>()) {
    bool v = boolVal.get<bool>();
}
```

---

### PropertySet

属性集合，管理组件的所有属性。

**头文件**: `include/aether/PropertySet.h`

```cpp
namespace aether {

class PropertySet {
public:
    // 构造函数
    PropertySet();
    
    // 析构函数
    ~PropertySet();
    
    // 复制构造函数
    PropertySet(const PropertySet& other);
    
    // 移动构造函数
    PropertySet(PropertySet&& other) noexcept;
    
    // 设置属性
    // 参数:
    //   id - 属性ID
    //   value - 属性值
    void setProperty(PropertyId id, const PropertyValue& value);
    
    // 获取属性
    // 参数:
    //   id - 属性ID
    // 返回: 属性值指针，如果不存在则返回nullptr
    PropertyValue* getProperty(PropertyId id);
    
    // 获取属性（const版本）
    const PropertyValue* getProperty(PropertyId id) const;
    
    // 检查是否有属性
    bool hasProperty(PropertyId id) const;
    
    // 移除属性
    void removeProperty(PropertyId id);
    
    // 清空所有属性
    void clear();
    
    // 获取属性数量
    size_t size() const;
    
    // 获取所有属性ID
    std::vector<PropertyId> getPropertyIds() const;
};

}
```

**使用示例**:

```cpp
auto* entry = storage->getComponent(button);

// 设置多个属性
entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
entry->properties.setProperty(PropertyId::Height, PropertyValue(50));
entry->properties.setProperty(PropertyId::Text, PropertyValue("Click Me"));
entry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1.0f));

// 获取属性
auto* width = entry->properties.getProperty(PropertyId::Width);
if (width) {
    if (width->is<int>()) {
        int w = width->get<int>();
    } else if (width->is<float>()) {
        float w = width->get<float>();
    }
}

// 检查属性存在
if (entry->properties.hasProperty(PropertyId::Text)) {
    // 属性存在
}

// 获取所有属性ID
auto ids = entry->properties.getPropertyIds();
for (auto id : ids) {
    // 处理每个属性
}
```

---

## 布局引擎API

### LayoutEngine

布局引擎，负责计算组件的位置和尺寸。

**头文件**: `include/aether/LayoutEngine.h`

```cpp
namespace aether {

// 布局引擎模式
enum class LayoutEngineMode {
    Normal,  // 正常模式
    Test     // 测试模式
};

// 脏标记结构
struct DirtyFlags {
    bool selfDirty;     // 自身脏标记
    bool childrenDirty; // 子组件脏标记
};

class LayoutEngine {
public:
    // 构造函数
    // 参数:
    //   storage - 组件存储引用
    explicit LayoutEngine(ComponentStorage& storage);
    
    // 析构函数
    ~LayoutEngine();
    
    // 设置布局模式
    void setMode(LayoutEngineMode mode);
    
    // 获取布局模式
    LayoutEngineMode getMode() const;
    
    // 标记组件为脏
    // 参数:
    //   h - 组件句柄
    void markDirty(ComponentHandle h);
    
    // 标记组件为脏（指定属性）
    // 参数:
    //   h - 组件句柄
    //   changedProp - 变化的属性ID
    void markDirty(ComponentHandle h, PropertyId changedProp);
    
    // 检查组件是否脏
    // 参数:
    //   h - 组件句柄
    // 返回: 是否需要重新布局
    bool isDirty(ComponentHandle h) const;
    
    // 按需重新布局（仅脏组件）
    void relayoutIfNeeded();
    
    // 强制重新布局（所有组件）
    void forceRelayout();
    
    // 清空脏标记
    void clearDirtyFlags();
    
    // 获取布局耗时
    float getTotalLayoutTime() const;
    
    // 重置统计信息
    void resetMetrics();
    
private:
    // 递归计算布局
    void computeLayout(int32_t nodeIdx);
    
    ComponentStorage& storage_;           // 组件存储引用
    std::vector<DirtyFlags> dirtyFlags_;  // 脏标记数组
    LayoutEngineMode mode_;               // 布局模式
    float totalLayoutTime_;              // 总布局耗时
};

}
```

**关键方法详解**:

#### markDirty

```cpp
// 示例1: 标记整个组件
engine->markDirty(button);

// 示例2: 标记特定属性变化
engine->markDirty(button, PropertyId::Width);
engine->markDirty(button, PropertyId::FlexGrow);
engine->markDirty(container, PropertyId::FlexDirection);
```

#### relayoutIfNeeded vs forceRelayout

```cpp
// 推荐：按需布局（更高效）
// 只重新布局受影响的组件
engine->markDirty(affectedComponent);
engine->relayoutIfNeeded();

// 适用于：初始化、窗口大小改变
// 强制布局所有组件
engine->forceRelayout();
```

#### isDirty

```cpp
// 检查组件是否需要重新布局
if (engine->isDirty(component)) {
    // 组件已脏，需要重新布局
}

// 在批量更新后检查
std::vector<ComponentHandle> changedComponents;
for (auto& change : changes) {
    engine->markDirty(change.component);
    if (engine->isDirty(change.component)) {
        changedComponents.push_back(change.component);
    }
}
```

**完整使用示例**:

```cpp
// 创建布局引擎
auto engine = std::make_unique<LayoutEngine>(*storage);

// 创建组件
auto container = storage->createComponent(ComponentType::Container, root);
auto* containerEntry = storage->getComponent(container);
containerEntry->layoutResult = {0, 0, 800, 600};
containerEntry->properties.setProperty(PropertyId::FlexDirection, 
                                      PropertyValue(FlexDirection::Row));

auto button1 = storage->createComponent(ComponentType::Button, container);
auto button2 = storage->createComponent(ComponentType::Button, container);

// 设置按钮属性
auto* btn1 = storage->getComponent(button1);
btn1->properties.setProperty(PropertyId::Width, PropertyValue(100));
btn1->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));

auto* btn2 = storage->getComponent(button2);
btn2->properties.setProperty(PropertyId::Width, PropertyValue(100));

// 计算布局
engine->forceRelayout();

// 检查结果
if (btn1->layoutResult.width > 100) {
    // 成功分配了剩余空间
}

// 模拟属性变化
btn1->properties.setProperty(PropertyId::Width, PropertyValue(200));
engine->markDirty(button1);
engine->relayoutIfNeeded();

// 获取性能统计
float layoutTime = engine->getTotalLayoutTime();
```

---

## 事件系统API

### EventDispatcher

事件分发器，处理用户交互事件。

**头文件**: `include/aether/EventDispatcher.h`

```cpp
namespace aether {

// 点结构
struct Point {
    float x;
    float y;
    
    Point() : x(0), y(0) {}
    Point(float px, float py) : x(px), y(py) {}
};

// 事件类型
enum class EventType {
    MouseMove,
    MouseDown,
    MouseUp,
    Click,
    DoubleClick,
    KeyDown,
    KeyUp,
    Focus,
    Blur
};

// 事件结构
struct Event {
    EventType type;
    Point position;
    ComponentHandle target;
    int keyCode;
    unsigned int modifiers;
};

class EventDispatcher {
public:
    // 构造函数
    // 参数:
    //   storage - 组件存储引用
    explicit EventDispatcher(ComponentStorage& storage);
    
    // 析构函数
    ~EventDispatcher();
    
    // 通知布局完成
    // 在布局计算完成后必须调用此方法
    void onLayoutComplete();
    
    // 命中测试
    // 参数:
    //   point - 测试点坐标
    // 返回: 命中的组件句柄，无命中则返回INVALID_COMPONENT_HANDLE
    ComponentHandle hitTest(const Point& point) const;
    
    // 分发事件
    // 参数:
    //   event - 事件结构
    void dispatchEvent(const Event& event);
    
    // 设置事件处理器
    template<typename Handler>
    void setEventHandler(EventType type, Handler&& handler);
    
    // 清除所有事件处理器
    void clearEventHandlers();
    
    // 获取最近点击的组件
    ComponentHandle getLastClicked() const;
    
    // 获取当前焦点组件
    ComponentHandle getFocusedComponent() const;
    
    // 设置焦点组件
    void setFocusedComponent(ComponentHandle component);
    
    // 清除焦点
    void clearFocus();
};

}
```

**关键方法详解**:

#### onLayoutComplete

```cpp
// 重要：在每次布局计算完成后必须调用
engine->forceRelayout();
dispatcher->onLayoutComplete();

// 或者
engine->relayoutIfNeeded();
dispatcher->onLayoutComplete();
```

#### hitTest

```cpp
// 示例1: 鼠标点击测试
Point clickPos = {mouseX, mouseY};
auto clicked = dispatcher->hitTest(clickPos);

if (clicked.isValid()) {
    auto* entry = storage->getComponent(clicked);
    if (entry && entry->enabled) {
        // 处理点击
    }
}

// 示例2: 连续命中测试
Point currentPos = GetMousePosition();
auto hovered = dispatcher->hitTest(currentPos);

if (hovered.isValid()) {
    if (hovered != lastHovered_) {
        // 鼠标进入新组件
        onMouseEnter(hovered);
        lastHovered_ = hovered;
    }
} else if (lastHovered_.isValid()) {
    // 鼠标离开组件
    onMouseLeave(lastHovered_);
    lastHovered_ = INVALID_COMPONENT_HANDLE;
}
```

#### dispatchEvent

```cpp
// 创建并分发点击事件
Event clickEvent;
clickEvent.type = EventType::Click;
clickEvent.position = {mouseX, mouseY};
clickEvent.target = dispatcher->hitTest({mouseX, mouseY});

dispatcher->dispatchEvent(clickEvent);
```

**完整使用示例**:

```cpp
// 创建事件分发器
auto dispatcher = std::make_unique<EventDispatcher>(*storage);

// 初始化布局
engine->forceRelayout();
dispatcher->onLayoutComplete();

// 处理Windows消息
while (GetMessage(&msg, NULL, 0, 0)) {
    if (msg.message == WM_LBUTTONDOWN) {
        Point clickPoint;
        clickPoint.x = LOWORD(msg.lParam);
        clickPoint.y = HIWORD(msg.lParam);
        
        auto clicked = dispatcher->hitTest(clickPoint);
        
        if (clicked.isValid()) {
            Event event;
            event.type = EventType::MouseDown;
            event.position = clickPoint;
            event.target = clicked;
            dispatcher->dispatchEvent(event);
        }
    }
    else if (msg.message == WM_MOUSEMOVE) {
        Point movePoint;
        movePoint.x = LOWORD(msg.lParam);
        movePoint.y = HIWORD(msg.lParam);
        
        auto hovered = dispatcher->hitTest(movePoint);
        
        if (hovered.isValid()) {
            Event event;
            event.type = EventType::MouseMove;
            event.position = movePoint;
            event.target = hovered;
            dispatcher->dispatchEvent(event);
        }
    }
    
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

---

## 渲染系统API

### Direct2DRenderer

Direct2D渲染器，负责将组件绘制到屏幕上。

**头文件**: `include/aether/Direct2DRenderer.h`

```cpp
namespace aether {

// 颜色结构
struct Color {
    float r;  // 红色 [0-1]
    float g;  // 绿色 [0-1]
    float b;  // 蓝色 [0-1]
    float a;  // 透明度 [0-1]
    
    // 构造函数
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float red, float green, float blue, float alpha = 1.0f) 
        : r(red), g(green), b(blue), a(alpha) {}
    
    // 从字节值创建
    static Color fromBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
    
    // 常用颜色
    static Color Black() { return Color(0, 0, 0); }
    static Color White() { return Color(1, 1, 1); }
    static Color Red() { return Color(1, 0, 0); }
    static Color Green() { return Color(0, 1, 0); }
    static Color Blue() { return Color(0, 0, 1); }
    static Color Transparent() { return Color(0, 0, 0, 0); }
};

class Direct2DRenderer {
public:
    // 构造函数
    // 参数:
    //   hwnd - 窗口句柄
    explicit Direct2DRenderer(HWND hwnd);
    
    // 析构函数
    ~Direct2DRenderer();
    
    // 初始化渲染器
    bool initialize();
    
    // 开始绘制
    void beginDraw();
    
    // 结束绘制
    void endDraw();
    
    // 清除画布
    void clear(const Color& color);
    
    // 绘制矩形
    void fillRect(const Rect& rect, const Color& color);
    
    // 绘制圆角矩形
    void fillRoundedRect(const Rect& rect, float radius, const Color& color);
    
    // 绘制矩形边框
    void drawRect(const Rect& rect, const Color& color, float strokeWidth = 1.0f);
    
    // 绘制文本
    // 参数:
    //   text - 文本内容（宽字符串）
    //   rect - 文本区域
    //   color - 文本颜色
    //   fontSize - 字体大小
    //   fontFamily - 字体家族
    void drawText(const std::wstring& text, const Rect& rect, 
                   const Color& color, float fontSize = 14.0f,
                   const wchar_t* fontFamily = L"Segoe UI");
    
    // 绘制位图
    void drawBitmap(const Rect& rect, ID2D1Bitmap* bitmap);
    
    // 获取D2D工厂
    ID2D1Factory* getFactory() const;
    
    // 获取渲染目标
    ID2D1HwndRenderTarget* getRenderTarget() const;
    
    // 调整大小
    void resize(int width, int height);
    
    // 获取DPI
    float getDpi() const;
    
    // 检查是否初始化成功
    bool isInitialized() const;
};

}
```

**关键方法详解**:

#### beginDraw/endDraw

```cpp
// 重要：必须成对使用
renderer->beginDraw();

// 绘制内容
renderer->clear(Color::White());

// ... 更多绘制代码

renderer->endDraw();
```

#### fillRect

```cpp
// 绘制填充矩形
Rect buttonRect = {100, 50, 120, 40};
Color buttonColor = Color::fromBytes(0, 120, 215);  // Windows蓝

renderer->fillRect(buttonRect, buttonColor);
```

#### fillRoundedRect

```cpp
// 绘制圆角按钮
Rect buttonRect = {100, 50, 120, 40};
renderer->fillRoundedRect(buttonRect, 8.0f, buttonColor);
```

#### drawText

```cpp
// 绘制按钮文本
Rect textRect = {100, 50, 120, 40};
std::wstring buttonText = L"Click Me";

renderer->drawText(buttonText, textRect, Color::White(), 14.0f);
```

**完整使用示例**:

```cpp
// 创建渲染器
auto renderer = std::make_unique<Direct2DRenderer>(hwnd);
renderer->initialize();

// 主渲染循环
void render() {
    renderer->beginDraw();
    
    // 清除画布
    renderer->clear(Color::White());
    
    // 绘制背景容器
    auto* rootEntry = storage->getRootEntry();
    renderer->fillRect(rootEntry->layoutResult, Color::fromBytes(240, 240, 240));
    
    // 绘制子组件
    for (int32_t childIdx : rootEntry->childrenIndices) {
        auto* child = storage->getComponentByIndex(childIdx);
        if (!child || !child->visible) continue;
        
        // 绘制组件背景
        if (child->type == ComponentType::Button) {
            // 按钮
            renderer->fillRoundedRect(
                child->layoutResult, 
                4.0f,
                Color::fromBytes(0, 120, 215)
            );
            
            // 绘制文本
            if (auto* text = child->properties.getProperty(PropertyId::Text)) {
                if (text->is<std::string>()) {
                    std::wstring wtext = StringToWString(text->get<std::string>());
                    renderer->drawText(
                        wtext,
                        child->layoutResult,
                        Color::White(),
                        14.0f
                    );
                }
            }
        }
        else if (child->type == ComponentType::TextField) {
            // 文本框
            renderer->fillRect(
                child->layoutResult,
                Color::White()
            );
            renderer->drawRect(
                child->layoutResult,
                Color::fromBytes(200, 200, 200),
                1.0f
            );
        }
    }
    
    renderer->endDraw();
}
```

---

## 状态管理API

### StateManager

状态管理器，用于保存和恢复应用状态。

**头文件**: `include/aether/StateManager.h`

```cpp
namespace aether {

class StateManager {
public:
    // 构造函数
    // 参数:
    //   storage - 组件存储引用
    explicit StateManager(ComponentStorage& storage);
    
    // 析构函数
    ~StateManager();
    
    // 创建快照
    std::unique_ptr<Snapshot> createSnapshot() const;
    
    // 恢复快照
    void restoreSnapshot(const Snapshot& snapshot);
    
    // 获取快照数量
    size_t getSnapshotCount() const;
    
    // 清空所有快照
    void clearSnapshots();
    
    // 注册组件到状态管理
    void registerComponent(ComponentHandle handle);
    
    // 取消注册组件
    void unregisterComponent(ComponentHandle handle);
    
    // 获取已注册组件数量
    size_t getRegisteredCount() const;
};

}
```

---

### Snapshot

快照，用于保存和恢复组件状态。

**头文件**: `include/aether/Snapshot.h`

```cpp
namespace aether {

class Snapshot {
public:
    // 构造函数
    // 参数:
    //   storage - 组件存储指针
    explicit Snapshot(ComponentStorage* storage);
    
    // 析构函数
    ~Snapshot();
    
    // 捕获当前状态
    void capture();
    
    // 恢复快照
    void restore();
    
    // 保存到文件
    // 参数:
    //   filename - 文件名
    // 返回: 是否成功
    bool saveToFile(const std::string& filename) const;
    
    // 从文件加载
    // 参数:
    //   filename - 文件名
    // 返回: 是否成功
    bool loadFromFile(const std::string& filename);
    
    // 保存到JSON字符串
    std::string toJSON() const;
    
    // 从JSON字符串加载
    void fromJSON(const std::string& json);
    
    // 获取快照时间戳
    int64_t getTimestamp() const;
    
    // 获取快照组件数量
    size_t getComponentCount() const;
    
    // 清空快照
    void clear();

private:
    ComponentStorage* storage_;  // 组件存储指针
    int64_t timestamp_;       // 时间戳
    std::vector<ComponentData> components_; // 组件数据
};

}
```

**使用示例**:

```cpp
// 创建快照管理器
auto stateManager = std::make_unique<StateManager>(*storage);

// 注册需要保存的组件
stateManager->registerComponent(component1);
stateManager->registerComponent(component2);

// 创建快照
auto snapshot = stateManager->createSnapshot();

// 修改组件
auto* entry = storage->getComponent(component1);
entry->properties.setProperty(PropertyId::Width, PropertyValue(200));

// 保存到文件
snapshot->saveToFile("autosave.json");

// 恢复快照
snapshot->restore();

// 或者手动操作
auto snapshot2 = std::make_unique<Snapshot>(storage.get());
snapshot2->capture();

// ... 用户操作 ...

// 保存快照
if (snapshot2->saveToFile("checkpoint.json")) {
    // 保存成功
}

// 加载快照
if (snapshot2->loadFromFile("checkpoint.json")) {
    snapshot2->restore();
}

// 获取JSON
std::string json = snapshot2->toJSON();

// 从JSON加载
snapshot2->fromJSON(json);
```

---

## 工具类和枚举

### FlexDirection

主轴方向枚举。

```cpp
namespace aether {

enum class FlexDirection {
    Row = 0,    // 水平方向，从左到右
    Column = 1  // 垂直方向，从上到下
};

}
```

---

### JustifyContent

主轴对齐方式枚举。

```cpp
namespace aether {

enum class JustifyContent {
    FlexStart = 0,     // 从主轴起点开始对齐
    FlexEnd = 1,       // 从主轴终点开始对齐
    Center = 2,        // 居中对齐
    SpaceBetween = 3,   // 两端对齐，项目之间间距相等
    SpaceAround = 4    // 每个项目两侧间距相等
};

}
```

---

### AlignItems

交叉轴对齐方式枚举。

```cpp
namespace aether {

enum class AlignItems {
    FlexStart = 0,   // 从交叉轴起点对齐
    FlexEnd = 1,    // 从交叉轴终点对齐
    Center = 2,     // 居中对齐
    Stretch = 3     // 拉伸到父容器尺寸
};

}
```

---

### FlexWrap

换行方式枚举。

```cpp
namespace aether {

enum class FlexWrap {
    NoWrap = 0,       // 不换行，所有项目在一行
    Wrap = 1,         // 换行
    WrapReverse = 2  // 反向换行
};

}
```

---

### Logger

日志系统。

**头文件**: `include/aether/Logger.h`

```cpp
namespace aether {

// 日志级别
enum class LogLevel {
    Trace = 0,  // 跟踪
    Debug = 1,  // 调试
    Info = 2,   // 信息
    Warn = 3,   // 警告
    Error = 4, // 错误
    Fatal = 5  // 致命错误
};

class Logger {
public:
    // 获取单例实例
    static Logger& getInstance();
    
    // 设置日志级别
    void setLevel(LogLevel level);
    
    // 获取日志级别
    LogLevel getLevel() const;
    
    // 设置日志文件
    void setLogFile(const std::string& filename);
    
    // 关闭日志文件
    void closeLogFile();
    
    // 日志输出方法
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    // 格式化日志
    void log(LogLevel level, const std::string& message);
    
    // 记录布局信息
    void logLayout(ComponentHandle handle, float x, float y, float width, float height);
    
    // 禁用实例
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    // 私有构造函数
    Logger();
    ~Logger();
    
    LogLevel level_;
    FILE* logFile_;
};

}
```

**使用示例**:

```cpp
// 获取日志实例
auto& logger = Logger::getInstance();

// 设置日志级别
logger.setLevel(LogLevel::Debug);

// 设置日志文件
logger.setLogFile("app.log");

// 输出日志
logger.info("Application started");
logger.debug("Component created: " + std::to_string(handle.index));
logger.warn("Low memory");
logger.error("Failed to load resource");

// 记录布局
logger.logLayout(buttonHandle, 100, 50, 120, 40);

// 关闭日志文件
logger.closeLogFile();
```

---

## 常量和类型定义

### 常用常量

```cpp
namespace aether {

// 无效的组件ID
const int32_t INVALID_COMPONENT_ID = -1;

// 默认尺寸
const float DEFAULT_WIDTH = 100.0f;
const float DEFAULT_HEIGHT = 30.0f;
const float DEFAULT_FONT_SIZE = 14.0f;

// 边距默认值
const float DEFAULT_MARGIN = 0.0f;
const float DEFAULT_PADDING = 0.0f;

// Flex默认值
const float DEFAULT_FLEX_GROW = 0.0f;
const float DEFAULT_FLEX_SHRINK = 1.0f;
const float DEFAULT_FLEX_BASIS = 0.0f;

// 渲染默认值
const float DEFAULT_BORDER_WIDTH = 1.0f;
const float DEFAULT_BORDER_RADIUS = 4.0f;

// 布局限制
const float MAX_LAYOUT_SIZE = 10000.0f;
const float MIN_LAYOUT_SIZE = 0.0f;

}
```

---

## 完整示例

### 示例1: 创建简单的按钮

```cpp
#include "aether/AetherApplication.h"
#include "aether/LayoutEngine.h"
#include "aether/Direct2DRenderer.h"

int main() {
    auto app = AetherApplication::create();
    auto& storage = app->getStorage();
    auto& engine = app->getLayoutEngine();
    auto& dispatcher = app->getEventDispatcher();
    
    // 创建容器
    auto container = storage.createComponent(ComponentType::Container);
    auto* containerEntry = storage.getComponent(container);
    containerEntry->layoutResult = {0, 0, 800, 600};
    containerEntry->properties.setProperty(
        PropertyId::FlexDirection, 
        PropertyValue(FlexDirection::Column)
    );
    
    // 创建按钮
    auto button = storage.createComponent(ComponentType::Button, container);
    auto* buttonEntry = storage.getComponent(button);
    buttonEntry->properties.setProperty(PropertyId::Width, PropertyValue(120));
    buttonEntry->properties.setProperty(PropertyId::Height, PropertyValue(40));
    buttonEntry->properties.setProperty(PropertyId::Text, PropertyValue("Click Me"));
    
    // 计算布局
    engine.forceRelayout();
    dispatcher.onLayoutComplete();
    
    // 创建渲染器
    auto renderer = std::make_unique<Direct2DRenderer>(app->getMainWindow());
    renderer->initialize();
    
    // 渲染
    renderer->beginDraw();
    renderer->clear(aether::Color::White());
    renderer->fillRoundedRect(
        buttonEntry->layoutResult,
        8.0f,
        aether::Color::fromBytes(0, 120, 215)
    );
    renderer->drawText(
        L"Click Me",
        buttonEntry->layoutResult,
        aether::Color::White()
    );
    renderer->endDraw();
    
    return app->run();
}
```

### 示例2: 响应式Flexbox布局

```cpp
void createFlexboxLayout(ComponentStorage& storage, LayoutEngine& engine) {
    // 主容器
    auto mainContainer = storage.createComponent(ComponentType::Container);
    auto* mainEntry = storage.getComponent(mainContainer);
    mainEntry->layoutResult = {0, 0, 800, 600};
    mainEntry->properties.setProperty(
        PropertyId::FlexDirection,
        PropertyValue(FlexDirection::Column)
    );
    
    // Header
    auto header = storage.createComponent(ComponentType::Container, mainContainer);
    auto* headerEntry = storage.getComponent(header);
    headerEntry->properties.setProperty(PropertyId::Height, PropertyValue(60));
    headerEntry->properties.setProperty(
        PropertyId::FlexDirection,
        PropertyValue(FlexDirection::Row)
    );
    
    // Logo
    auto logo = storage.createComponent(ComponentType::Label, header);
    auto* logoEntry = storage.getComponent(logo);
    logoEntry->properties.setProperty(PropertyId::Width, PropertyValue(50));
    logoEntry->properties.setProperty(PropertyId::Text, PropertyValue("Logo"));
    
    // 导航
    auto nav = storage.createComponent(ComponentType::Container, header);
    auto* navEntry = storage.getComponent(nav);
    navEntry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));
    navEntry->properties.setProperty(
        PropertyId::FlexDirection,
        PropertyValue(FlexDirection::Row)
    );
    navEntry->properties.setProperty(
        PropertyId::JustifyContent,
        PropertyValue(JustifyContent::FlexEnd)
    );
    
    // 导航项
    const char* navItems[] = {"Home", "About", "Contact"};
    for (int i = 0; i < 3; i++) {
        auto navItem = storage.createComponent(ComponentType::Button, nav);
        auto* navItemEntry = storage.getComponent(navItem);
        navItemEntry->properties.setProperty(PropertyId::Width, PropertyValue(80));
        navItemEntry->properties.setProperty(PropertyId::MarginLeft, PropertyValue(10));
        navItemEntry->properties.setProperty(
            PropertyId::Text,
            PropertyValue(navItems[i])
        );
    }
    
    // Content
    auto content = storage.createComponent(ComponentType::Container, mainContainer);
    auto* contentEntry = storage.getComponent(content);
    contentEntry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));
    contentEntry->properties.setProperty(
        PropertyId::PaddingLeft,
        PropertyValue(20)
    );
    contentEntry->properties.setProperty(
        PropertyId::PaddingRight,
        PropertyValue(20)
    );
    contentEntry->properties.setProperty(
        PropertyId::PaddingTop,
        PropertyValue(20)
    );
    
    // Footer
    auto footer = storage.createComponent(ComponentType::Container, mainContainer);
    auto* footerEntry = storage.getComponent(footer);
    footerEntry->properties.setProperty(PropertyId::Height, PropertyValue(40));
    footerEntry->properties.setProperty(PropertyId::Text, PropertyValue("© 2024"));
    
    // 计算布局
    engine.forceRelayout();
}
```

---

**文档版本**: 1.0.0
**最后更新**: 2024-05-24
**API版本**: v1.0.0
