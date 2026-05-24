# Aether UI 引擎用户手册

## 📖 目录

1. [概述](#概述)
2. [快速开始](#快速开始)
3. [核心概念](#核心概念)
4. [组件系统](#组件系统)
5. [布局引擎](#布局引擎)
6. [事件系统](#事件系统)
7. [渲染系统](#渲染系统)
8. [状态管理](#状态管理)
9. [最佳实践](#最佳实践)
10. [常见问题](#常见问题)

---

## 概述

### 什么是Aether UI引擎？

Aether UI 是一个基于C++的现代UI框架，采用Flexbox布局算法，支持Windows Direct2D渲染。它被设计用于构建高性能、响应式的桌面应用程序。

### 主要特性

- **Flexbox布局系统**: 现代化的布局算法，支持水平/垂直排列
- **组件化架构**: 基于组件树的数据驱动UI
- **Direct2D渲染**: Windows原生高性能2D图形渲染
- **事件驱动**: 完整的事件系统和命中测试
- **状态管理**: 内置状态管理和快照功能
- **单元测试**: 完整的测试覆盖（153个测试用例）

---

## 快速开始

### 环境要求

- Windows 10/11
- Visual Studio 2019 或更高版本
- CMake 3.15+
- Windows SDK 10.0+

### 安装和构建

```bash
# 克隆项目
git clone <repository-url>
cd aether-ui

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
cmake --build . --config Debug

# 运行测试
ctest -C Debug
```

### 第一个Aether应用

```cpp
#include "aether/AetherApplication.h"
#include "aether/LayoutEngine.h"
#include "aether/ComponentStorage.h"

int main() {
    // 创建应用实例
    auto app = AetherApplication::create();
    
    // 创建组件存储和布局引擎
    auto storage = std::make_unique<ComponentStorage>();
    auto layoutEngine = std::make_unique<LayoutEngine>(*storage);
    
    // 创建根组件
    auto root = storage->createComponent(ComponentType::Container);
    auto* rootEntry = storage->getComponent(root);
    rootEntry->layoutResult = {0, 0, 800, 600};
    rootEntry->properties.setProperty(PropertyId::FlexDirection, 
                                     PropertyValue(FlexDirection::Row));
    
    // 创建按钮
    auto button = storage->createComponent(ComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    buttonEntry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    buttonEntry->properties.setProperty(PropertyId::Height, PropertyValue(50));
    buttonEntry->properties.setProperty(PropertyId::Text, PropertyValue("Click Me"));
    
    // 计算布局
    layoutEngine->forceRelayout();
    
    // 渲染
    app->render();
    
    return 0;
}
```

---

## 核心概念

### 组件（Component）

组件是Aether UI的基本构建块。每个组件都有：
- **唯一标识符**: 通过ComponentHandle访问
- **属性集**: 存储组件的配置
- **布局结果**: 组件的位置和尺寸
- **父子关系**: 形成组件树

```cpp
// 创建组件
auto button = storage->createComponent(ComponentType::Button, parent);

// 获取组件
auto* entry = storage->getComponent(button);

// 设置属性
entry->properties.setProperty(PropertyId::Width, PropertyValue(200));
entry->properties.setProperty(PropertyId::Height, PropertyValue(50));

// 组件类型
enum class ComponentType {
    Container,    // 容器
    Button,      // 按钮
    TextField,   // 文本输入框
    Label,       // 标签
    Checkbox     // 复选框
};
```

### 组件句柄（ComponentHandle）

组件句柄用于安全地引用组件：

```cpp
// 组件句柄结构
struct ComponentHandle {
    int32_t index;      // 组件索引
    int32_t generation; // 组件代数（用于检测悬空引用）
    
    bool isValid() const;  // 检查句柄是否有效
    bool operator==(const ComponentHandle& other) const;
};

// 安全访问
if (handle.isValid()) {
    auto* entry = storage->getComponent(handle);
    // ...
}
```

### 属性系统（Property）

属性系统使用类型安全的变体存储：

```cpp
// 支持的属性类型
PropertyValue value;

// 整数属性
value = PropertyValue(100);

// 浮点属性
value = PropertyValue(50.5f);

// 枚举属性
value = PropertyValue(FlexDirection::Row);
value = PropertyValue(JustifyContent::Center);

// 字符串属性
value = PropertyValue("Hello World");

// 设置属性
entry->properties.setProperty(PropertyId::Width, value);

// 获取属性
if (auto* prop = entry->properties.getProperty(PropertyId::Width)) {
    if (prop->is<int>()) {
        int width = prop->get<int>();
    }
}
```

---

## 组件系统

### 组件类型详解

#### 1. 容器（Container）

容器用于组织其他组件，支持Flexbox布局：

```cpp
auto container = storage->createComponent(ComponentType::Container, parent);
auto* entry = storage->getComponent(container);

// 设置为水平布局
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Row));

// 设置为垂直布局
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Column));

// 设置内边距
entry->properties.setProperty(PropertyId::PaddingLeft, PropertyValue(10.0f));
entry->properties.setProperty(PropertyId::PaddingTop, PropertyValue(10.0f));
```

#### 2. 按钮（Button）

按钮组件支持交互：

```cpp
auto button = storage->createComponent(ComponentType::Button, parent);
auto* entry = storage->getComponent(button);

// 基本属性
entry->properties.setProperty(PropertyId::Width, PropertyValue(120));
entry->properties.setProperty(PropertyId::Height, PropertyValue(40));
entry->properties.setProperty(PropertyId::Text, PropertyValue("Submit"));

// 样式
entry->properties.setProperty(PropertyId::BackgroundColor, 
                             PropertyValue(Color{255, 0, 0, 255}));
entry->properties.setProperty(PropertyId::TextColor, 
                             PropertyValue(Color{255, 255, 255, 255}));
```

#### 3. 文本输入框（TextField）

```cpp
auto textField = storage->createComponent(ComponentType::TextField, parent);
auto* entry = storage->getComponent(textField);

entry->properties.setProperty(PropertyId::Width, PropertyValue(200));
entry->properties.setProperty(PropertyId::Height, PropertyValue(30));
entry->properties.setProperty(PropertyId::PlaceholderText, 
                             PropertyValue("Enter text..."));
```

### 组件关系管理

```cpp
// 添加子组件
auto child = storage->createComponent(ComponentType::Button, parent);

// 移除子组件
storage->removeComponent(child);

// 获取父组件
auto parent = storage->getParent(child);

// 遍历子组件
for (auto childIdx : entry->childrenIndices) {
    auto* childEntry = storage->getComponentByIndex(childIdx);
    // ...
}
```

---

## 布局引擎

### Flexbox布局

Aether使用Flexbox布局算法，这是现代UI框架中最灵活的布局系统。

### 主轴和交叉轴

```
FlexDirection::Row (水平):
    主轴: 水平方向 (→)
    交叉轴: 垂直方向 (↓)

FlexDirection::Column (垂直):
    主轴: 垂直方向 (↓)
    交叉轴: 水平方向 (→)
```

### 关键属性

#### 1. FlexDirection（主轴方向）

```cpp
// 水平排列
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Row));

// 垂直排列
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Column));
```

#### 2. FlexGrow（放大因子）

FlexGrow决定当有剩余空间时，组件如何分配空间：

```cpp
// 示例：三个组件按 1:2:1 比例分配剩余空间
// 容器宽度: 600px
// 子组件基础宽度: 100px × 3 = 300px
// 剩余空间: 600 - 300 = 300px

auto child1 = storage->createComponent(ComponentType::Button, container);
auto* entry1 = storage->getComponent(child1);
entry1->properties.setProperty(PropertyId::Width, PropertyValue(100));
entry1->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1)); // 获取 75px

auto child2 = storage->createComponent(ComponentType::Button, container);
auto* entry2 = storage->getComponent(child2);
entry2->properties.setProperty(PropertyId::Width, PropertyValue(100));
entry2->properties.setProperty(PropertyId::FlexGrow, PropertyValue(2)); // 获取 150px

auto child3 = storage->createComponent(ComponentType::Button, container);
auto* entry3 = storage->getComponent(child3);
entry3->properties.setProperty(PropertyId::Width, PropertyValue(100));
entry3->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1)); // 获取 75px

// 最终宽度: 175, 250, 175
```

#### 3. FlexShrink（收缩因子）

当空间不足时，FlexShrink决定组件如何收缩：

```cpp
// 示例：三个组件宽度各300px，容器宽度600px
// 溢出空间: 900 - 600 = 300px
// 按 1:2:1 比例收缩

auto child1 = storage->createComponent(ComponentType::Button, container);
auto* entry1 = storage->getComponent(child1);
entry1->properties.setProperty(PropertyId::Width, PropertyValue(300));
entry1->properties.setProperty(PropertyId::FlexShrink, PropertyValue(1)); // 收缩 75px

auto child2 = storage->createComponent(ComponentType::Button, container);
auto* entry2 = storage->getComponent(child2);
entry2->properties.setProperty(PropertyId::Width, PropertyValue(300));
entry2->properties.setProperty(PropertyId::FlexShrink, PropertyValue(2)); // 收缩 150px

auto child3 = storage->createComponent(ComponentType::Button, container);
auto* entry3 = storage->getComponent(child3);
entry3->properties.setProperty(PropertyId::Width, PropertyValue(300));
entry3->properties.setProperty(PropertyId::FlexShrink, PropertyValue(1)); // 收缩 75px

// 最终宽度: 225, 150, 225
```

#### 4. FlexBasis（基准尺寸）

FlexBasis用作组件的初始尺寸，优先级高于Width/Height：

```cpp
auto button = storage->createComponent(ComponentType::Button, container);
auto* entry = storage->getComponent(button);

// FlexBasis 优先于 Width
entry->properties.setProperty(PropertyId::Width, PropertyValue(100));      // 被忽略
entry->properties.setProperty(PropertyId::FlexBasis, PropertyValue(200.0f)); // 使用200px
entry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(0));
```

#### 5. 外边距（Margin）

```cpp
auto button = storage->createComponent(ComponentType::Button, container);
auto* entry = storage->getComponent(button);

entry->properties.setProperty(PropertyId::MarginLeft, PropertyValue(10.0f));
entry->properties.setProperty(PropertyId::MarginTop, PropertyValue(20.0f));
entry->properties.setProperty(PropertyId::MarginRight, PropertyValue(10.0f));
entry->properties.setProperty(PropertyId::MarginBottom, PropertyValue(20.0f));
```

#### 6. 内边距（Padding）

```cpp
auto container = storage->createComponent(ComponentType::Container, parent);
auto* entry = storage->getComponent(container);

entry->properties.setProperty(PropertyId::PaddingLeft, PropertyValue(16.0f));
entry->properties.setProperty(PropertyId::PaddingTop, PropertyValue(12.0f));
entry->properties.setProperty(PropertyId::PaddingRight, PropertyValue(16.0f));
entry->properties.setProperty(PropertyId::PaddingBottom, PropertyValue(12.0f));
```

### 布局计算

布局引擎会在以下情况自动计算布局：

1. 调用 `forceRelayout()` - 强制重新布局所有组件
2. 调用 `relayoutIfNeeded()` - 只重新布局脏标记的组件
3. 属性变化时自动标记脏状态

```cpp
// 创建布局引擎
auto engine = std::make_unique<LayoutEngine>(*storage);

// 强制重新布局
engine->forceRelayout();

// 或者按需布局（更高效）
engine->markDirty(someComponent, PropertyId::Width);
engine->relayoutIfNeeded();

// 检查组件是否需要重新布局
if (engine->isDirty(component)) {
    // ...
}
```

### 布局示例

#### 示例1：水平居中布局

```
[    [Button1]  [Button2]  ]
```

```cpp
auto container = storage->createComponent(ComponentType::Container, root);
auto* entry = storage->getComponent(container);

entry->layoutResult = {0, 0, 800, 100};
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Row));
entry->properties.setProperty(PropertyId::JustifyContent, 
                             PropertyValue(JustifyContent::Center));

auto button1 = storage->createComponent(ComponentType::Button, container);
auto* btn1 = storage->getComponent(button1);
btn1->properties.setProperty(PropertyId::Width, PropertyValue(100));
btn1->properties.setProperty(PropertyId::Height, PropertyValue(40));

auto button2 = storage->createComponent(ComponentType::Button, container);
auto* btn2 = storage->getComponent(button2);
btn2->properties.setProperty(PropertyId::Width, PropertyValue(100));
btn2->properties.setProperty(PropertyId::Height, PropertyValue(40));
```

#### 示例2：垂直堆叠布局

```
[Button1]
[Button2]
[Button3]
```

```cpp
auto container = storage->createComponent(ComponentType::Container, root);
auto* entry = storage->getComponent(container);

entry->layoutResult = {0, 0, 200, 300};
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Column));

// 自动堆叠
auto button1 = storage->createComponent(ComponentType::Button, container);
auto button2 = storage->createComponent(ComponentType::Button, container);
auto button3 = storage->createComponent(ComponentType::Button, container);
```

#### 示例3：FlexGrow填充剩余空间

```
[Fixed][    Fill    ]
```

```cpp
auto container = storage->createComponent(ComponentType::Container, root);
auto* entry = storage->getComponent(container);

entry->layoutResult = {0, 0, 800, 100};
entry->properties.setProperty(PropertyId::FlexDirection, 
                             PropertyValue(FlexDirection::Row));

// 固定宽度的按钮
auto fixed = storage->createComponent(ComponentType::Button, container);
auto* fixedEntry = storage->getComponent(fixed);
fixedEntry->properties.setProperty(PropertyId::Width, PropertyValue(150));

// 填充剩余空间的按钮
auto fill = storage->createComponent(ComponentType::Button, container);
auto* fillEntry = storage->getComponent(fill);
fillEntry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));
```

---

## 事件系统

### 事件分发器（EventDispatcher）

事件分发器处理用户交互，如点击、悬停等：

```cpp
// 创建事件分发器
auto dispatcher = std::make_unique<EventDispatcher>(*storage);

// 通知布局完成（必须调用才能进行命中测试）
dispatcher->onLayoutComplete();

// 命中测试：检查某个点是否在组件上
Point clickPoint = {150, 125};
auto hitResult = dispatcher->hitTest(clickPoint);

if (hitResult.isValid()) {
    auto* clickedEntry = storage->getComponent(hitResult);
    // 处理点击事件
}
```

### 组件状态

```cpp
auto button = storage->createComponent(ComponentType::Button, parent);
auto* entry = storage->getComponent(button);

// 可见性
entry->visible = true;  // 默认true

// 启用状态
entry->enabled = true;  // 默认true，禁用后不响应交互

// 焦点
entry->focused = false; // 默认false
```

### 事件监听

```cpp
// 在应用层处理事件
void handleClick(ComponentHandle clicked) {
    auto* entry = storage->getComponent(clicked);
    
    if (entry->type == ComponentType::Button) {
        // 处理按钮点击
    }
}
```

---

## 渲染系统

### Direct2D渲染器

Aether使用Windows Direct2D进行高性能2D渲染：

```cpp
#include "aether/Direct2DRenderer.h"

// 创建渲染器
auto renderer = std::make_unique<Direct2DRenderer>(hwnd);

// 开始绘制
renderer->beginDraw();

// 绘制组件
for (auto childIdx : rootEntry->childrenIndices) {
    auto* child = storage->getComponentByIndex(childIdx);
    if (child && child->visible) {
        renderer->fillRect(child->layoutResult, child->properties);
    }
}

// 结束绘制
renderer->endDraw();
```

### 绘制组件

```cpp
// 绘制矩形
Rect rect = {100, 100, 200, 50};
Color fillColor = {255, 100, 100, 255};  // RGBA
renderer->fillRect(rect, fillColor);

// 绘制圆角矩形
renderer->fillRoundedRect(rect, 8.0f, fillColor);

// 绘制文本
std::wstring text = L"Button Text";
renderer->drawText(text, rect, textColor);

// 绘制边框
Color strokeColor = {0, 0, 0, 255};
renderer->drawRect(rect, strokeColor, 1.0f);
```

---

## 状态管理

### 状态管理器

状态管理器用于保存和恢复应用状态：

```cpp
#include "aether/StateManager.h"

auto stateManager = std::make_unique<StateManager>(*storage);

// 保存快照
auto snapshot = stateManager->createSnapshot();

// 恢复快照
stateManager->restoreSnapshot(snapshot);
```

### 快照系统

```cpp
// 创建快照
auto snapshot = std::make_unique<Snapshot>(storage.get());
snapshot->capture();

// 保存到文件
snapshot->saveToFile("state.json");

// 从文件加载
snapshot->loadFromFile("state.json");

// 恢复状态
snapshot->restore();
```

---

## 最佳实践

### 1. 合理的组件层级

**✅ 推荐**

```cpp
// 扁平化结构，减少嵌套
Root
├── Header
│   ├── Logo
│   └── Navigation
├── Content
│   ├── Sidebar
│   └── Main
└── Footer
```

**❌ 避免**

```cpp
// 避免过深的嵌套
Root
└── Container
    └── Container
        └── Container
            └── Container  // 太深！
                └── Button
```

### 2. 使用FlexGrow/FlexShrink而非固定尺寸

**✅ 推荐**

```cpp
// 使用flex-grow让组件自动填充
entry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));

// 或使用flex-shrink让组件可收缩
entry->properties.setProperty(PropertyId::FlexShrink, PropertyValue(1));
```

**❌ 避免**

```cpp
// 避免完全固定尺寸
entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
entry->properties.setProperty(PropertyId::Height, PropertyValue(50));
```

### 3. 批量更新后统一布局

**✅ 推荐**

```cpp
// 批量设置属性
entry1->properties.setProperty(PropertyId::Width, PropertyValue(200));
entry1->properties.setProperty(PropertyId::Height, PropertyValue(50));

entry2->properties.setProperty(PropertyId::Width, PropertyValue(150));
entry2->properties.setProperty(PropertyId::Height, PropertyValue(50));

// 统一触发布局
engine->forceRelayout();
```

**❌ 避免**

```cpp
// 不要每次属性改变都触发布局
entry1->properties.setProperty(PropertyId::Width, PropertyValue(200));
engine->forceRelayout();  // 浪费！

entry1->properties.setProperty(PropertyId::Height, PropertyValue(50));
engine->forceRelayout();  // 浪费！

entry2->properties.setProperty(PropertyId::Width, PropertyValue(150));
engine->forceRelayout();  // 浪费！
```

### 4. 使用脏标记优化

```cpp
// 只标记受影响的组件
engine->markDirty(button, PropertyId::Width);

// 使用按需布局
engine->relayoutIfNeeded();  // 比 forceRelayout 更高效
```

### 5. 组件句柄安全使用

**✅ 推荐**

```cpp
// 总是检查句柄有效性
if (handle.isValid()) {
    auto* entry = storage->getComponent(handle);
    if (entry) {
        // 安全使用
    }
}
```

**❌ 避免**

```cpp
// 不要假设句柄总是有效的
auto* entry = storage->getComponent(handle);  // 可能返回nullptr
```

### 6. 内存管理

```cpp
// 使用智能指针管理生命周期
auto storage = std::make_unique<ComponentStorage>();
auto engine = std::make_unique<LayoutEngine>(*storage);

// 或者使用引用但确保生命周期
ComponentStorage storage;
LayoutEngine engine(storage);  // engine引用storage，storage必须先存在
```

---

## 常见问题

### Q1: 为什么组件不显示？

**可能原因：**

1. **尺寸为0或负数**
   ```cpp
   // 检查layoutResult
   if (entry->layoutResult.width <= 0 || 
       entry->layoutResult.height <= 0) {
       // 需要设置正确的尺寸
       entry->layoutResult = {x, y, 100, 50};
   }
   ```

2. **visible = false**
   ```cpp
   entry->visible = true;  // 确保可见
   ```

3. **组件不在可见区域内**
   ```cpp
   // 确保组件在窗口内
   entry->layoutResult = {0, 0, 100, 50};  // 不要设置为负坐标
   ```

4. **未调用forceRelayout**
   ```cpp
   engine->forceRelayout();  // 确保布局已计算
   ```

### Q2: FlexGrow不生效？

**检查清单：**

1. ✅ 父容器有明确的尺寸
   ```cpp
   parentEntry->layoutResult = {0, 0, 800, 600};  // 必须设置
   ```

2. ✅ 子组件有基础尺寸或flex-basis
   ```cpp
   childEntry->properties.setProperty(PropertyId::Width, PropertyValue(100));
   ```

3. ✅ 容器设置了FlexDirection
   ```cpp
   parentEntry->properties.setProperty(PropertyId::FlexDirection, 
                                      PropertyValue(FlexDirection::Row));
   ```

4. ✅ FlexGrow > 0
   ```cpp
   childEntry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));
   ```

### Q3: 布局计算出错？

**调试方法：**

1. **启用日志**
   ```cpp
   Logger::getInstance().setLevel(LogLevel::Debug);
   ```

2. **检查脏标记**
   ```cpp
   if (engine->isDirty(component)) {
       // 组件需要重新布局
   }
   ```

3. **强制重新布局**
   ```cpp
   engine->forceRelayout();  // 确保一致性
   ```

### Q4: 事件不响应？

**检查清单：**

1. ✅ 调用了onLayoutComplete()
   ```cpp
   dispatcher->onLayoutComplete();  // 必须调用
   ```

2. ✅ 组件可见且启用
   ```cpp
   entry->visible = true;
   entry->enabled = true;
   ```

3. ✅ 组件有正确的尺寸
   ```cpp
   // 确保组件不为零尺寸
   if (entry->layoutResult.width <= 0 || 
       entry->layoutResult.height <= 0) {
       engine->forceRelayout();
   }
   ```

### Q5: 性能问题？

**优化建议：**

1. **使用relayoutIfNeeded而非forceRelayout**
   ```cpp
   // ✅ 推荐：只重新布局脏组件
   engine->markDirty(affected);
   engine->relayoutIfNeeded();
   
   // ❌ 避免：总是重新布局所有组件
   engine->forceRelayout();
   ```

2. **减少组件数量**
   ```cpp
   // 使用容器组织组件，而不是创建大量独立组件
   ```

3. **避免频繁的属性更新**
   ```cpp
   // 批量更新
   for (auto& change : changes) {
       storage->getComponent(change.handle)
           ->properties.setProperty(change.prop, change.value);
   }
   engine->relayoutIfNeeded();  // 一次布局
   ```

---

## 附录

### 属性ID列表

| 属性ID | 类型 | 说明 |
|--------|------|------|
| Width | int/float | 组件宽度 |
| Height | int/float | 组件高度 |
| FlexDirection | FlexDirection | 主轴方向 |
| FlexGrow | float | 放大因子 |
| FlexShrink | float | 收缩因子 |
| FlexBasis | float | 基准尺寸 |
| MarginLeft | float | 左外边距 |
| MarginTop | float | 上外边距 |
| MarginRight | float | 右外边距 |
| MarginBottom | float | 下外边距 |
| PaddingLeft | float | 左内边距 |
| PaddingTop | float | 上内边距 |
| PaddingRight | float | 右内边距 |
| PaddingBottom | float | 下内边距 |
| BackgroundColor | Color | 背景颜色 |
| TextColor | Color | 文本颜色 |
| Text | string | 文本内容 |
| PlaceholderText | string | 占位文本 |

### 枚举值

**FlexDirection:**
```cpp
enum class FlexDirection {
    Row = 0,      // 水平，从左到右
    Column = 1    // 垂直，从上到下
};
```

**JustifyContent:**
```cpp
enum class JustifyContent {
    FlexStart = 0,    // 起点对齐
    FlexEnd = 1,      // 终点对齐
    Center = 2,       // 居中对齐
    SpaceBetween = 3, // 两端对齐
    SpaceAround = 4   // 环绕对齐
};
```

**AlignItems:**
```cpp
enum class AlignItems {
    FlexStart = 0,   // 交叉轴起点对齐
    FlexEnd = 1,     // 交叉轴终点对齐
    Center = 2,       // 交叉轴居中对齐
    Stretch = 3      // 拉伸到父容器尺寸
};
```

**FlexWrap:**
```cpp
enum class FlexWrap {
    NoWrap = 0,      // 不换行
    Wrap = 1,        // 换行
    WrapReverse = 2 // 反向换行
};
```

---

## 更新日志

### v1.0.0 (2024-05-24)
- ✨ 完成Flexbox布局引擎实现
- ✨ 添加153个单元测试
- ✨ 实现Direct2D渲染器
- ✨ 完善事件系统
- ✨ 添加状态管理和快照功能
- ✨ 完整的文档和用户手册

---

## 许可证

本项目采用 MIT 许可证 - 详见 LICENSE 文件

## 联系方式

- GitHub: https://github.com/yourusername/aether-ui
- 邮箱: your.email@example.com

---

**祝你使用愉快！🎉**
