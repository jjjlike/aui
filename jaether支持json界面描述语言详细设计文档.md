# jaether支持JSON界面描述语言详细设计文档

> **版本**：1.0.0  
> **目标**：在 jaether 框架中实现基于 A2UI（Agent-to-UI）协议的 JSON 界面描述语言，使界面能够从 JSON 初始化、动态生成，并支持双向转换。  
> **设计原则**：A2UI 协议兼容、最小侵入、渐进式渲染、LLM 友好、与现有框架融合统一。

---

## 目录

1. [概述与动机](#1-概述与动机)
2. [A2UI 协议概述](#2-a2ui-协议概述)
3. [总体架构设计](#3-总体架构设计)
4. [A2UI 组件目录设计](#4-a2ui-组件目录设计)
5. [核心模块详细设计](#5-核心模块详细设计)
    - 5.1 [JA2UIParser — JSON 到组件引擎](#51-ja2uiparser--json-到组件引擎)
    - 5.2 [JA2UIGenerator — 组件到 JSON 引擎](#52-ja2uigenerator--组件到-json-引擎)
    - 5.3 [JJSurfaceManager — Surface 生命周期管理](#53-jjsurfacemanager--surface-生命周期管理)
    - 5.4 [JJDataModel — 数据模型与绑定](#54-jjdatamodel--数据模型与绑定)
6. [与现有框架的融合](#6-与现有框架的融合)
7. [简单控件实现与测试用例设计](#7-简单控件实现与测试用例设计)
8. [增量更新与流式渲染](#8-增量更新与流式渲染)
9. [实现计划](#9-实现计划)
10. [附录](#10-附录)

---

## 1. 概述与动机

### 1.1 背景

jaether 框架已具备完整的逻辑层能力：组件存储（ComponentStorage）、Flexbox 布局引擎（LayoutEngine）、事件分发器（EventDispatcher）、快照系统（Snapshot）以及 JSON 解析器（JSONParser）。现有框架可以通过 C++ API 编程式地创建和操作 UI 组件。

在现代 AI Agent 驱动的应用场景中，需要一种**声明式的、语言无关的**界面描述方式，使得：
- AI Agent（LLM）可以**直接生成 UI**，无需编写 C++ 代码
- 界面结构可以在运行时**动态加载和修改**
- 同一套描述规则支持**双向转换**（JSON → 组件 ↔ 组件 → JSON）

### 1.2 设计目标

| 目标 | 指标 |
|------|------|
| A2UI 协议兼容 | 支持 v0.8 `surfaceUpdate` / v0.9 `updateComponents` 消息格式 |
| 双向转换 | JSON → 组件树，组件树 → JSON 使用同一套描述规则 |
| 最小侵入 | 新增模块，不修改现有 ComponentStorage、LayoutEngine 核心逻辑 |
| 渐进式渲染 | 支持 JSONL 流式解析，增量构建 UI |
| LLM 友好 | 平铺邻接表结构，字符串 ID，避免深层嵌套 |
| 可测试 | 支持无头模式，JSON 初始化和验证 |

### 1.3 核心原则

```
┌────────────────────────────────────────────────────────────────┐
│                     jaether 框架 (现有)                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
│  │ Component    │  │ Layout       │  │ Event               │  │
│  │ Storage      │  │ Engine       │  │ Dispatcher           │  │
│  └──────┬───────┘  └──────┬───────┘  └──────────┬───────────┘  │
│         │                 │                      │              │
│         └─────────────────┼──────────────────────┘              │
│                           │                                     │
│  ┌────────────────────────┼──────────────────────────────────┐  │
│  │         新增：A2UI 协议层（不修改现有模块）                │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌───────────────────┐ │  │
│  │  │ A2UIParser  │  │A2UIGenerator│  │  SurfaceManager   │ │  │
│  │  │ JSON→组件树  │  │ 组件树→JSON │  │  Surface生命周期   │ │  │
│  │  └──────┬──────┘  └──────┬──────┘  └────────┬──────────┘ │  │
│  │         │                │                   │            │  │
│  │         └────────────────┼───────────────────┘            │  │
│  │                          ▼                                │  │
│  │               JLogicLayer (现有统一接口)                   │  │
│  └───────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
```

---

## 2. A2UI 协议概述

### 2.1 协议版本选择

jaether 采用 **A2UI v0.9** 作为主要参考版本，同时兼容 v0.8 格式。选择 v0.9 的理由：

- v0.9 的扁平组件结构（`"component": "Text"` 而非嵌套 `{"Text": {...}}`）更简洁
- 子节点直接使用字符串数组而非 `{"explicitList": [...]}`
- 字符串值直接使用原始类型（`"Hello"` 而非 `{"literalString": "Hello"}`）
- v0.9 正成为社区主流，生态更完善

### 2.2 消息类型映射

A2UI 定义了四种服务器→客户端消息类型，jaether 作为 UI 渲染端（Client），需要解析这些消息：

| A2UI 消息 (v0.9) | jaether 模块 | 说明 |
|---|---|---|
| `createSurface` | `JJSurfaceManager` | 创建渲染表面，指定目录 |
| `updateComponents` | `JA2UIParser` | 创建/更新组件定义 |
| `updateDataModel` | `JJDataModel` | 更新数据模型 |
| `deleteSurface` | `JJSurfaceManager` | 删除表面 |

此外，jaether 作为双向系统，还需要 `JA2UIGenerator` 将组件树导出为 A2UI JSON，这覆盖客户端→服务器的 `userAction` 场景。

### 2.3 邻接表模型

A2UI 使用**平铺邻接表**而非嵌套树表示组件层次结构，这是其最核心的设计特征：

```json
{
  "version": "v0.9",
  "updateComponents": {
    "surfaceId": "main",
    "components": [
      { "id": "root",     "component": "Column", "children": ["header", "body"] },
      { "id": "header",   "component": "Text",   "text": "我的待办列表" },
      { "id": "body",     "component": "Row",    "children": ["input", "add-btn"] },
      { "id": "input",    "component": "TextField", "label": "输入待办" },
      { "id": "add-btn",  "component": "Button", "child": "btn-text", "action": {"event": {"name": "add"}} },
      { "id": "btn-text", "component": "Text",   "text": "添加" }
    ]
  }
}
```

**优势**：
- LLM 可以逐步生成组件，无需在单次推理中完成整棵嵌套树
- 通过 ID 即可精确定位和更新任意组件
- 天然支持增量更新和流式渲染

### 2.4 数据绑定（BoundValue）

A2UI 支持两种值传递方式：

| 方式 | 示例 | 说明 |
|------|------|------|
| **字面量** | `"text": "Hello"` | 固定值，直接渲染 |
| **数据绑定** | `"text": {"path": "/user/name"}` | 从 DataModel 动态读取 |

jaether 在 JSON→组件时，字面量直接设置为属性值，数据绑定值建立数据模型监听。

---

## 3. 总体架构设计

### 3.1 模块分层

```
┌─────────────────────────────────────────────────────────────────┐
│                    应用层 / AI Agent                             │
│  JSON 界面描述 ←→ jaether JLogicLayer ←→ 原生渲染 (Direct2D)    │
└─────────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────────┐
│               A2UI 协议层 (新增)                                 │
│                                                                  │
│  ┌──────────────────┐  ┌──────────────────┐                     │
│  │   JA2UIParser    │  │  JA2UIGenerator  │                     │
│  │  JSON → 组件树    │  │  组件树 → JSON   │                     │
│  │  - parseSurface   │  │  - generateJSON  │                     │
│  │  - parseComponent │  │  - toA2UIMessage │                     │
│  │  - applyUpdate    │  │  - toSnapshot    │                     │
│  └────────┬─────────┘  └────────┬─────────┘                     │
│           │                     │                                │
│  ┌────────┴─────────────────────┴─────────┐                     │
│  │           JJSurfaceManager             │                     │
│  │  - Surface 生命周期 (create/delete)     │                     │
│  │  - 组件 ID → JComponentHandle 映射     │                     │
│  │  - 增量更新调度                         │                     │
│  └────────┬───────────────────────────────┘                     │
│           │                                                     │
│  ┌────────┴─────────┐  ┌────────────────────┐                  │
│  │   JJDataModel    │  │  JJComponentCatalog │                  │
│  │  - 数据模型存储   │  │  - 组件类型注册表   │                  │
│  │  - 路径解析       │  │  - 属性校验         │                  │
│  │  - 绑定通知       │  │  - 默认值填充       │                  │
│  └──────────────────┘  └────────────────────┘                  │
└─────────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────────┐
│               jaether 核心层 (现有，不修改)                      │
│                                                                  │
│  JComponentStorage  JLayoutEngine  JEventDispatcher              │
│  JStateManager      JSnapshot       JTestController             │
│  JJSONParser         JLogicLayer                                │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 数据流

#### JSON → 组件（加载界面）

```
JSON 字符串
    │
    ▼
JJSONParser::parse()  ──────→  JJSONValue (通用 JSON 树)
    │
    ▼
JA2UIParser::parseSurfaceJson()
    │
    ├── 遍历 components[] 数组
    ├── 每项调用 parseComponentDescriptor()
    │     ├── 读取 "id", "component" (类型)
    │     ├── 读取组件属性 (text, children, action...)
    │     └── 返回 A2UIComponentDescriptor
    │
    ▼
JA2UIParser::applyDescriptors()
    │
    ├── 按拓扑序创建 JComponentHandle (调用 JLogicLayer::createComponent)
    ├── 设置属性 (调用 JLogicLayer::setProperty)
    ├── 建立父子关系
    └── 建立 id→handle 映射 (存入 SurfaceManager)
    │
    ▼
JLogicLayer::runFrame()  ──→  布局计算 → 渲染就绪
```

#### 组件 → JSON（导出界面）

```
JLogicLayer (组件树状态)
    │
    ▼
JA2UIGenerator::generate()
    │
    ├── 遍历 JComponentStorage::forEach()
    ├── 对每个组件调用 generateComponentJSON()
    │     ├── 读取 type → 映射为 A2UI 组件类型字符串
    │     ├── 读取 properties → 映射为 A2UI 属性
    │     ├── 读取 childrenIndices → 映射为 children ID 数组
    │     └── 输出 A2UI JSON 对象
    │
    ▼
更新到 Snapshot 系统 (JTestController::getComponentTreeJSON)
    │
    ▼
A2UI JSON 字符串
```

### 3.3 双向一致性保证

```
         parseSurfaceJson()
JSON ────────────────────────→ JComponentHandle 树
  ↑                                    │
  │                                    │
  └────────────────────────────────────┘
         generateSurfaceJson()

断言: generateSurfaceJson(parseSurfaceJson(json)) ≈ json (归一化后)
```

---

## 4. A2UI 组件目录设计

### 4.1 jaether 内置目录（jaether-basic-catalog）

基于 A2UI Basic Catalog，jaether 提供以下内置组件：

| A2UI 组件 | jaether JComponentType | 类别 | 说明 |
|---|---|---| 
| `Text` | `JComponentType::Text` | 显示组件 | 文本展示，支持 variant (h1-h5, body, caption) |
| `Button` | `JComponentType::Button` | 交互组件 | 按钮，支持 action 和 child |
| `TextField` | `JComponentType::Input` | 交互组件 | 文本输入，支持 label 和 textFieldType |
| `Row` | `JComponentType::Container` | 布局组件 | 水平排列 (flexDirection=Row) |
| `Column` | `JComponentType::Container` | 布局组件 | 垂直排列 (flexDirection=Column) |
| `Card` | `JComponentType::Container` | 容器组件 | 带边框和内边距的容器 |
| `CheckBox` | `JComponentType::Container` | 交互组件 | 复选框，映射为 Container+Button 组合 |

### 4.2 组件属性映射表

#### Text 组件

| A2UI 属性 | jaether JPropertyId | 类型 | 说明 |
|---|---|---|---|
| `text` | `JPropertyId::Text` | string / BoundValue | 文本内容 |
| `variant` | `JPropertyId::FontSize` | string→float | h1=32, h2=28, h3=24, h4=20, h5=16, body=14, caption=12 |
| `usageHint` (v0.8) | `JPropertyId::FontSize` | string→float | 同 variant |
| `weight` | `JPropertyId::FlexGrow` | float | flex-grow 权重 |

#### Button 组件

| A2UI 属性 | jaether JPropertyId | 类型 | 说明 |
|---|---|---|---|
| `child` | 子组件关系 | string (ID) | 按钮内的子组件（通常是 Text） |
| `action` | `JPropertyId::UserDefined+0` | JSON object | 动作描述，存储为 JSON 字符串 |
| `variant` (v0.9) | `JPropertyId::BackgroundColor` | string→Color | primary/danger 等映射为颜色 |
| `primary` (v0.8) | `JPropertyId::BackgroundColor` | bool | true→蓝色背景 |
| `weight` | `JPropertyId::FlexGrow` | float | flex-grow 权重 |

#### TextField 组件

| A2UI 属性 | jaether JPropertyId | 类型 | 说明 |
|---|---|---|---|
| `label` | `JPropertyId::Text` | string | 输入框标签 |
| `value` / `text` | `JPropertyId::UserDefined+1` | string / BoundValue | 当前输入值 |
| `textFieldType` | `JPropertyId::UserDefined+2` | string | shortText/longText/number/obscured/date |
| `weight` | `JPropertyId::FlexGrow` | float | flex-grow 权重 |

#### Row / Column 布局组件

| A2UI 属性 | jaether JPropertyId | 类型 | 说明 |
|---|---|---|---|
| `children` | 子组件关系 | string[] / template | 子组件 ID 列表 |
| `justify` (v0.9) | `JPropertyId::JJustifyContent` | string→enum | start/center/end/spaceBetween/spaceAround |
| `distribution` (v0.8) | `JPropertyId::JJustifyContent` | string→enum | 同 justify |
| `align` (v0.9) | `JPropertyId::JAlignItems` | string→enum | start/center/end/stretch |
| `alignment` (v0.8) | `JPropertyId::JAlignItems` | string→enum | 同 align |
| `weight` | `JPropertyId::FlexGrow` | float | flex-grow 权重 |

#### Card 组件

| A2UI 属性 | jaether JPropertyId | 类型 | 说明 |
|---|---|---|---|
| `child` | 子组件关系 | string (ID) | 卡片内子组件 |
| `weight` | `JPropertyId::FlexGrow` | float | flex-grow 权重 |

### 4.3 组件目录注册机制

```cpp
class JJComponentCatalog {
public:
    struct ComponentTypeInfo {
        std::string a2uiName;           // A2UI 类型名，如 "Text"
        JComponentType jaetherType;    // jaether 内部类型
        bool isLayout;                  // 是否为布局容器
        std::vector<std::string> requiredProps;  // 必填属性
        std::vector<std::string> optionalProps;  // 可选属性
    };

    // 注册组件类型
    void registerType(const ComponentTypeInfo& info);
    
    // 查询 A2UI 类型名 → JComponentType
    JComponentType getType(const std::string& a2uiName) const;
    
    // 查询 JComponentType → A2UI 类型名
    std::string getA2UIName(JComponentType type) const;
    
    // 检查属性是否对该类型有效
    bool isValidProperty(const std::string& a2uiName, const std::string& propName) const;
    
    // 获取默认属性值
    JJSONValue getDefaultValue(const std::string& a2uiName, const std::string& propName) const;
    
    // 获取属性到 JPropertyId 的映射
    JPropertyId getPropertyId(const std::string& a2uiName, const std::string& propName) const;

private:
    std::unordered_map<std::string, ComponentTypeInfo> catalog_;
    std::unordered_map<JComponentType, std::string> reverseMap_;
};
```

### 4.4 属性类型强制转换映射

| A2UI JSON 类型 | JPropertyValue 类型 | 转换规则 |
|---|---|---|
| `string` | `std::string` | 直接转换 |
| `number` (整数) | `int` | 直接转换 |
| `number` (浮点) | `float` | 直接转换 |
| `boolean` | `bool` | 直接转换 |
| `{"path": "/..."}` | `std::string` (存储路径) | 标记为数据绑定 |
| `"h1"` (variant) | `float` (字号) | variant→fontSize 映射 |
| `"start"` (align) | `JAlignItems::FlexStart` | 字符串→枚举映射 |
| `{"event": {"name": "..."}}` | `std::string` (JSON) | 整个 action 对象序列化 |

---

## 5. 核心模块详细设计

### 5.1 JA2UIParser — JSON 到组件引擎

#### 5.1.1 类接口

```cpp
class JA2UIParser {
public:
    explicit JA2UIParser(JLogicLayer& logicLayer);

    // 解析完整的 surface JSON 并创建所有组件
    // @param json A2UI 格式的 JSON 字符串（可以是单个消息或 JSONL 多行）
    // @param surfaceId 目标 surface ID
    // @return 创建的根组件 ID
    std::string parseAndApply(const std::string& json, const std::string& surfaceId);

    // 解析单条 updateComponents 消息
    // @param updateJson 单条消息的 JSON 字符串
    // @param surfaceId 目标 surface ID
    void applyComponentUpdate(const std::string& updateJson, const std::string& surfaceId);

    // 解析单条 updateDataModel 消息
    // @param dataJson 数据模型更新的 JSON 字符串
    // @param surfaceId 目标 surface ID
    void applyDataModelUpdate(const std::string& dataJson, const std::string& surfaceId);

    // 解析 JSONL（多行，每行一条消息）
    void parseJSONL(const std::string& jsonl);

private:
    // 内部：解析单个组件描述符
    struct ComponentDescriptor {
        std::string id;                      // A2UI 组件 ID
        std::string type;                    // A2UI 组件类型名
        std::vector<std::string> children;   // 子组件 ID 列表
        JJSONObject properties;              // 属性键值对（含数据绑定标记）
        bool hasDataBinding = false;         // 是否有数据绑定属性
    };
    
    ComponentDescriptor parseDescriptor(const JJSONObject& obj);

    // 将 ComponentDescriptor 转换为 jaether 组件
    JComponentHandle createComponent(const ComponentDescriptor& desc, 
                                      JComponentHandle parent);

    // 设置组件的单个属性
    void applyProperty(JComponentHandle handle, 
                       const std::string& propName, 
                       const JJSONValue& propValue);

    // 递归创建组件树（拓扑排序，叶子先创建）
    void buildComponentTree(const std::vector<ComponentDescriptor>& descriptors,
                            const std::string& surfaceId);

    JLogicLayer& logicLayer_;
    std::unique_ptr<JJComponentCatalog> catalog_;
    std::unique_ptr<JJSurfaceManager> surfaceManager_;
    std::unique_ptr<JJDataModel> dataModel_;
};
```

#### 5.1.2 解析流程

```text
Algorithm: JA2UIParser::parseAndApply
Input: json (A2UI JSON string), surfaceId
Output: root component ID string

1. 使用 JJSONParser::parse() 解析 JSON
2. 区分消息类型:
   a. 若为 {"updateComponents": {...}} (v0.9)
      提取 components[] 数组
   b. 若为 {"surfaceUpdate": {...}} (v0.8)
      提取 components[] 数组，转换为 v0.9 格式
   c. 若为 {"updateDataModel": {...}}
      调用 applyDataModelUpdate()
   d. 若为 {"createSurface": {...}}
      创建新 Surface
3. 对于 components[] 中的每个元素:
   a. 调用 parseDescriptor() 解析为 ComponentDescriptor
   b. 校验组件类型存在于 catalog
   c. 校验必填属性存在
4. 对 descriptors 进行拓扑排序（叶子先创建，父后创建）
5. 按序调用 createComponent() 创建 JComponentHandle
6. 设置属性 (applyProperty)
7. 建立 id→handle 映射 (SurfaceManager)
8. 调用 logicLayer_.runFrame() 触发布局计算
9. 返回根组件 ID
```

#### 5.1.3 数据绑定处理

对于绑定值 `{"path": "/user/name"}`，解析器：

```
1. 识别 "path" 键 → 标记为数据绑定
2. 存储绑定路径: bindings_[handle]["propName"] = "/user/name"
3. 监听 DataModel 该路径的变更
4. 当 DataModel 变更时，自动调用 setProperty 更新组件
```

#### 5.1.4 v0.8 兼容层

```cpp
// 将 v0.8 组件描述符转换为 v0.9 格式
JJSONObject convertV08ToV09(const JJSONObject& v08Component) {
    JJSONObject v09;
    
    // 提取类型键名 (Text, Button, Row...)
    auto& v08Props = /* v08Component["component"]["Text"] */;
    
    // 转换：{"literalString": "hello"} → "hello"
    // 转换：{"path": "/data"} → {"path": "/data"} (不变)
    // 转换：{"explicitList": [...]} → [...]
    // 转换：usageHint → variant
    
    return v09;
}
```

---

### 5.2 JA2UIGenerator — 组件到 JSON 引擎

#### 5.2.1 类接口

```cpp
class JA2UIGenerator {
public:
    explicit JA2UIGenerator(const JLogicLayer& logicLayer);

    // 生成完整 surface 的 A2UI JSON
    // @param surfaceId 目标 surface ID
    // @return A2UI updateComponents 消息 JSON
    std::string generateSurfaceJSON(const std::string& surfaceId) const;

    // 生成单个组件的 A2UI JSON
    // @param handle 组件句柄
    // @param a2uiId 组件的 A2UI ID
    // @return A2UI 组件描述 JSON
    JJSONObject generateComponentJSON(JComponentHandle handle, 
                                       const std::string& a2uiId) const;

    // 生成数据模型的 A2UI JSON
    std::string generateDataModelJSON(const std::string& surfaceId) const;

    // 与 TestController 集成：覆盖 getComponentTreeJSON
    // 输出 A2UI 格式的组件树 JSON
    std::string getComponentTreeA2UI() const;

private:
    // 将 JPropertyValue 转换为 A2UI JSON 值
    JJSONValue propertyToA2UIValue(JPropertyId id, 
                                    const JPropertyValue& value) const;

    // 将子组件索引列表转换为 A2UI ID 字符串列表
    std::vector<std::string> childrenToA2UIIDs(
        const std::vector<int32_t>& childrenIndices) const;

    const JLogicLayer& logicLayer_;
    std::unique_ptr<JJComponentCatalog> catalog_;
};
```

#### 5.2.2 生成流程

```text
Algorithm: JA2UIGenerator::generateSurfaceJSON
Input: surfaceId
Output: A2UI JSON string

1. 获取 surface 的根组件 handle 和所有组件列表
2. 对每个组件:
   a. 获取 type → 映射为 A2UI 类型名 (catalog_.getA2UIName)
   b. 获取 properties → 映射为 A2UI 属性键值对
      - Text → "text"
      - Width/Height → "width"/"height" (单位: DIP)
      - FlexGrow → "weight"
      - BackgroundColor → 根据颜色推断 variant
      - 等等
   c. 获取 childrenIndices → 查询 id→handle 映射，转为 A2UI ID 数组
   d. 构造 A2UI 组件对象
3. 包装为 updateComponents 消息
4. 序列化为 JSON 字符串
```

#### 5.2.3 属性逆向映射表

| JPropertyId | A2UI 属性名 | 取值转换规则 |
|---|---|---|
| `Text` | `text` | 直接输出 string |
| `Width` | `width` | 直接输出 float |
| `Height` | `height` | 直接输出 float |
| `FlexGrow` | `weight` | 直接输出 float |
| `JustifyContent` | `justify` | 枚举→字符串 |
| `AlignItems` | `align` | 枚举→字符串 |
| `FontSize` | `variant` | 字号→variant 名称 (32→h1, 24→h3...) |
| `BackgroundColor` | `variant` (Button) | 颜色→primary/danger |
| `PaddingLeft/Right/Top/Bottom` | `padding` | 合并为对象 |
| `MarginLeft/Right/Top/Bottom` | `margin` | 合并为对象 |

---

### 5.3 JJSurfaceManager — Surface 生命周期管理

#### 5.3.1 类接口

```cpp
class JJSurfaceManager {
public:
    explicit JJSurfaceManager(JLogicLayer& logicLayer);

    // 创建新的 surface
    // @param surfaceId 唯一 surface ID
    // @param catalogId 目录 ID（默认 "jaether-basic"）
    // @return 是否创建成功
    bool createSurface(const std::string& surfaceId, 
                       const std::string& catalogId = "jaether-basic");

    // 删除 surface 及其所有组件
    void deleteSurface(const std::string& surfaceId);

    // 注册组件 ID 映射
    // @param surfaceId surface ID
    // @param a2uiId A2UI 组件 ID (如 "header")
    // @param handle jaether 组件句柄
    void registerComponent(const std::string& surfaceId,
                           const std::string& a2uiId,
                           JComponentHandle handle);

    // 注销组件
    void unregisterComponent(const std::string& surfaceId,
                             const std::string& a2uiId);

    // 通过 A2UI ID 查找组件句柄
    JComponentHandle findComponent(const std::string& surfaceId,
                                    const std::string& a2uiId) const;

    // 通过组件句柄查找 A2UI ID
    std::string findA2UIId(const std::string& surfaceId,
                           JComponentHandle handle) const;

    // 获取 surface 的根组件句柄
    JComponentHandle getRootComponent(const std::string& surfaceId) const;

    // 获取 surface 中所有组件的 A2UI ID 列表
    std::vector<std::string> getAllComponentIds(const std::string& surfaceId) const;

    // 检查 surface 是否存在
    bool hasSurface(const std::string& surfaceId) const;

private:
    struct Surface {
        std::string id;
        std::string catalogId;
        JComponentHandle rootHandle;
        std::unordered_map<std::string, JComponentHandle> a2uiToHandle;
        std::unordered_map<int32_t, std::string> handleToA2UI;
    };
    
    JLogicLayer& logicLayer_;
    std::unordered_map<std::string, Surface> surfaces_;
};
```

---

### 5.4 JJDataModel — 数据模型与绑定

#### 5.4.1 类接口

```cpp
class JJDataModel {
public:
    explicit JJDataModel(JLogicLayer& logicLayer);

    // 应用数据模型更新
    // @param surfaceId 目标 surface
    // @param path 更新路径（如 "user" 或 "/user/email"）
    // @param contents 数据条目（key-value 数组）
    void applyUpdate(const std::string& surfaceId,
                     const std::string& path,
                     const JJSONArray& contents);

    // 获取数据模型中的值
    JJSONValue getValue(const std::string& surfaceId,
                         const std::string& path) const;

    // 注册组件属性的数据绑定
    void bindProperty(const std::string& surfaceId,
                      JComponentHandle handle,
                      JPropertyId propId,
                      const std::string& dataPath);

    // 导出数据模型为 A2UI JSON
    std::string exportJSON(const std::string& surfaceId) const;

    // 当数据模型变更时通知所有绑定组件
    void notifyBindings(const std::string& surfaceId, 
                        const std::string& changedPath);

private:
    struct DataModelNode {
        JJSONValue value;
        std::unordered_map<std::string, DataModelNode> children;
    };

    struct PropertyBinding {
        JComponentHandle handle;
        JPropertyId propId;
    };

    // 解析路径 "/a/b/c" 为 ["a", "b", "c"]
    std::vector<std::string> parsePath(const std::string& path) const;

    // 在数据模型中设置值（支持路径遍历）
    void setValueAtPath(const std::string& surfaceId,
                        const std::vector<std::string>& pathSegments,
                        const JJSONValue& value);

    JLogicLayer& logicLayer_;
    
    // surfaceId → 数据模型根节点
    std::unordered_map<std::string, DataModelNode> dataModels_;
    
    // surfaceId → (path → 绑定列表)
    std::unordered_map<std::string, 
        std::unordered_map<std::string, std::vector<PropertyBinding>>> bindings_;
};
```

---

## 6. 与现有框架的融合

### 6.1 JLogicLayer 集成

在 JLogicLayer 中新增 A2UI 相关接口，不修改现有方法：

```cpp
class JLogicLayer {
public:
    // --- 现有接口 (不变) ---
    JComponentHandle createComponent(JComponentType type, JComponentHandle parent = {});
    void destroyComponent(JComponentHandle handle);
    void setProperty(JComponentHandle h, JPropertyId id, JPropertyValue value);
    // ...

    // --- 新增 A2UI 接口 ---

    // 从 A2UI JSON 加载界面
    // @param json A2UI 格式 JSON 字符串
    // @param surfaceId surface ID，默认 "main"
    // @return 根组件的 A2UI ID
    std::string loadFromA2UI(const std::string& json, 
                              const std::string& surfaceId = "main");

    // 导出为 A2UI JSON
    std::string exportToA2UI(const std::string& surfaceId = "main") const;

    // 流式添加组件（渐进式渲染）
    void streamComponent(const std::string& json, 
                         const std::string& surfaceId = "main");

    // 获取 A2UI 解析器
    JA2UIParser& getA2UIParser();
    const JA2UIParser& getA2UIParser() const;

    // 获取 A2UI 生成器
    JA2UIGenerator& getA2UIGenerator();
    const JA2UIGenerator& getA2UIGenerator() const;

    // 获取 Surface 管理器
    JJSurfaceManager& getSurfaceManager();
    const JJSurfaceManager& getSurfaceManager() const;

private:
    // 现有成员 (不变)
    JComponentStorage storage_;
    JLayoutEngine layoutEngine_;
    JEventDispatcher eventDispatcher_;
    JStateManager stateManager_;
    JTestController testController_;

    // 新增成员 (惰性初始化，不影响无 A2UI 场景的性能)
    std::unique_ptr<JA2UIParser> a2uiParser_;
    std::unique_ptr<JA2UIGenerator> a2uiGenerator_;
    std::unique_ptr<JJSurfaceManager> surfaceManager_;
    std::unique_ptr<JJDataModel> dataModel_;
};
```

### 6.2 与 Snapshot 系统的统一

现有的 `JSnapshotSerializer::capture()` 和 `JTestController::getComponentTreeJSON()` 输出的组件树 JSON 格式与 A2UI 不同。通过以下方式统一：

```cpp
// 方式一：A2UI 格式快照
std::string JLogicLayer::exportToA2UI() const {
    return a2uiGenerator_->generateSurfaceJSON("main");
}

// 方式二：测试快照仍用原有格式（向后兼容）
std::string JTestController::getComponentTreeJSON() {
    // 保持现有逻辑不变
}

// 方式三：新增 A2UI 格式快照
std::string JTestController::getComponentTreeA2UI() {
    return a2uiGenerator_->getComponentTreeA2UI();
}
```

### 6.3 与现有 JSONParser 的关系

- **JJSONParser**：通用 JSON 解析器，负责将 JSON 字符串解析为 `JJSONValue` 树，**保持不变**
- **JA2UIParser**：A2UI 语义解析器，使用 `JJSONParser` 解析 JSON，然后按 A2UI 协议语义创建组件，**新增**

```
JJSONValue (已解析)
    │
    ├── 通用场景: 直接使用 JJSONValue 对象
    │
    └── A2UI 场景: JA2UIParser 解析为组件树
```

### 6.4 与现有 JSONPatch 的关系

现有的 `JJSONPatch` 类骨架可以复用作为增量更新机制的基础。A2UI 的 `updateComponents` 天然支持增量更新：发送已存在 ID 的组件描述即表示更新。

---

## 7. 简单控件实现与测试用例设计

### 7.1 第一阶段实现的控件

| 控件 | A2UI 类型 | jaether 实现方式 |
|------|----------|-----------------|
| 文本显示 | `Text` | `JComponentType::Text`，支持 variant→fontSize 映射 |
| 按钮 | `Button` | `JComponentType::Button` + 子 Text 组件 |
| 文本输入 | `TextField` | `JComponentType::Input`，存储 label/value |
| 水平布局 | `Row` | `JComponentType::Container` + `FlexDirection::Row` |
| 垂直布局 | `Column` | `JComponentType::Container` + `FlexDirection::Column` |
| 卡片容器 | `Card` | 新增 `JComponentType::Card` 或使用 Container + 样式 |
| 复选框 | `CheckBox` | Container 包装 Button + Text 组合 |

### 7.2 Card 组件类型扩展

为支持 Card 组件，在 `JComponentType` 枚举中新增：

```cpp
enum class JComponentType : uint8_t {
    Container,
    Button,
    Text,
    Input,
    ScrollView,
    Image,
    Card,      // 新增：卡片容器
    Custom
};
```

在 `Direct2DRenderer` 渲染端新增 Card 绘制逻辑（填充背景 + 边框 + padding）。

### 7.3 测试用例设计

#### 测试 1：JSON 到组件的基本解析

```cpp
TEST(A2UIParserTest, ParseTextComponent) {
    JLogicLayer logicLayer;
    JA2UIParser parser(logicLayer);
    
    const char* json = R"({
        "version": "v0.9",
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["title"]},
                {"id": "title", "component": "Text", "text": "Hello A2UI", "variant": "h1"}
            ]
        }
    })";
    
    std::string rootId = parser.parseAndApply(json, "main");
    EXPECT_EQ(rootId, "root");
    
    JComponentHandle titleHandle = logicLayer.getSurfaceManager()
                                        .findComponent("main", "title");
    EXPECT_TRUE(titleHandle.isValid());
    
    auto* textProp = logicLayer.getProperty(titleHandle, JPropertyId::Text);
    ASSERT_NE(textProp, nullptr);
    EXPECT_EQ(textProp->get<std::string>(), "Hello A2UI");
}
```

#### 测试 2：布局组件的 children 正确建立父子关系

```cpp
TEST(A2UIParserTest, RowWithChildren) {
    JLogicLayer logicLayer;
    JA2UIParser parser(logicLayer);
    
    const char* json = R"({
        "version": "v0.9",
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Row", "children": ["btn1", "btn2", "btn3"]},
                {"id": "btn1", "component": "Button", "child": "txt1"},
                {"id": "txt1", "component": "Text", "text": "Btn 1"},
                {"id": "btn2", "component": "Button", "child": "txt2"},
                {"id": "txt2", "component": "Text", "text": "Btn 2"},
                {"id": "btn3", "component": "Button", "child": "txt3"},
                {"id": "txt3", "component": "Text", "text": "Btn 3"}
            ]
        }
    })";
    
    parser.parseAndApply(json, "main");
    
    // 验证：btn1, btn2, btn3 应该是 root 的子组件
    auto rootHandle = logicLayer.getSurfaceManager()
                       .findComponent("main", "root");
    auto* rootEntry = logicLayer.getStorage().getComponent(rootHandle);
    ASSERT_NE(rootEntry, nullptr);
    EXPECT_EQ(rootEntry->childrenIndices.size(), 3);
    
    // 验证：txt1 应该是 btn1 的子组件
    auto btn1Handle = logicLayer.getSurfaceManager()
                      .findComponent("main", "btn1");
    auto* btn1Entry = logicLayer.getStorage().getComponent(btn1Handle);
    ASSERT_NE(btn1Entry, nullptr);
    EXPECT_EQ(btn1Entry->childrenIndices.size(), 1);
}
```

#### 测试 3：组件树到 JSON 的导出

```cpp
TEST(A2UIGeneratorTest, GenerateFromComponents) {
    JLogicLayer logicLayer;
    
    // 用 C++ API 创建组件
    auto root = logicLayer.createComponent(JComponentType::Container);
    logicLayer.setProperty(root, JPropertyId::FlexDirection, 
                           JPropertyValue(static_cast<int>(JFlexDirection::Column)));
    
    auto text = logicLayer.createComponent(JComponentType::Text, root);
    logicLayer.setProperty(text, JPropertyId::Text, 
                           JPropertyValue(std::string("Hello")));
    
    auto btn = logicLayer.createComponent(JComponentType::Button, root);
    logicLayer.setProperty(btn, JPropertyId::Text, 
                           JPropertyValue(std::string("Click")));
    
    logicLayer.runFrame();
    
    // 注册到 SurfaceManager
    auto& sm = logicLayer.getSurfaceManager();
    sm.createSurface("test");
    sm.registerComponent("test", "root", root);
    sm.registerComponent("test", "greeting", text);
    sm.registerComponent("test", "action-btn", btn);
    
    // 导出为 A2UI JSON
    JA2UIGenerator generator(logicLayer);
    std::string a2uiJson = generator.generateSurfaceJSON("test");
    
    // 验证：导出的 JSON 应该能被重新解析
    JA2UIParser parser(logicLayer);
    std::string newRootId = parser.parseAndApply(a2uiJson, "reimported");
    EXPECT_EQ(newRootId, "root");
    
    // 验证组件数量一致
    EXPECT_EQ(sm.getAllComponentIds("reimported").size(), 3);
}
```

#### 测试 4：增量更新

```cpp
TEST(A2UIParserTest, IncrementalUpdate) {
    JLogicLayer logicLayer;
    JA2UIParser parser(logicLayer);
    
    // 第一步：创建初始界面
    parser.parseAndApply(R"({
        "version": "v0.9",
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["label"]},
                {"id": "label", "component": "Text", "text": "Initial"}
            ]
        }
    })", "main");
    
    // 验证初始文本
    auto labelHandle = logicLayer.getSurfaceManager()
                       .findComponent("main", "label");
    auto* textProp = logicLayer.getProperty(labelHandle, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "Initial");
    
    // 第二步：增量更新 — 修改已有组件的属性
    parser.applyComponentUpdate(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "label", "component": "Text", "text": "Updated"}
            ]
        }
    })", "main");
    
    // 验证文本已更新
    textProp = logicLayer.getProperty(labelHandle, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "Updated");
}
```

#### 测试 5：待办列表应用通过 JSON 初始化

```cpp
TEST(A2UIIntegrationTest, TodoAppFromJSON) {
    JLogicLayer logicLayer;
    
    const char* todoJSON = R"({
        "version": "v0.9",
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {
                    "id": "root",
                    "component": "Column",
                    "children": ["title", "input-row", "list", "stats"]
                },
                {
                    "id": "title",
                    "component": "Text",
                    "text": "我的待办列表",
                    "variant": "h1"
                },
                {
                    "id": "input-row",
                    "component": "Row",
                    "children": ["todo-input", "add-btn"]
                },
                {
                    "id": "todo-input",
                    "component": "TextField",
                    "label": "输入待办事项",
                    "textFieldType": "shortText"
                },
                {
                    "id": "add-btn",
                    "component": "Button",
                    "child": "add-text",
                    "variant": "primary",
                    "action": {"event": {"name": "add_todo"}}
                },
                {
                    "id": "add-text",
                    "component": "Text",
                    "text": "添加"
                },
                {
                    "id": "list",
                    "component": "Column",
                    "children": []
                },
                {
                    "id": "stats",
                    "component": "Text",
                    "text": "总计: 0 | 已完成: 0 | 未完成: 0"
                }
            ]
        }
    })";
    
    logicLayer.loadFromA2UI(todoJSON);
    
    auto& sm = logicLayer.getSurfaceManager();
    
    // 验证关键组件存在
    EXPECT_TRUE(sm.findComponent("main", "root").isValid());
    EXPECT_TRUE(sm.findComponent("main", "title").isValid());
    EXPECT_TRUE(sm.findComponent("main", "add-btn").isValid());
    EXPECT_TRUE(sm.findComponent("main", "todo-input").isValid());
    
    // 验证布局正确
    auto rootHandle = sm.findComponent("main", "root");
    auto* rootEntry = logicLayer.getStorage().getComponent(rootHandle);
    EXPECT_EQ(rootEntry->childrenIndices.size(), 4); // title, input-row, list, stats
}
```

#### 测试 6：双向转换幂等性

```cpp
TEST(A2UIRoundtripTest, ParseThenGenerateThenParse) {
    JLogicLayer logicLayer;
    
    const char* originalJSON = R"({...同测试1的JSON...})";
    
    // 第一次解析
    logicLayer.loadFromA2UI(originalJSON, "surface1");
    auto& sm = logicLayer.getSurfaceManager();
    size_t count1 = sm.getAllComponentIds("surface1").size();
    
    // 导出为 JSON
    std::string exported = logicLayer.exportToA2UI("surface1");
    
    // 重新解析
    logicLayer.loadFromA2UI(exported, "surface2");
    size_t count2 = sm.getAllComponentIds("surface2").size();
    
    // 组件数量应该一致
    EXPECT_EQ(count1, count2);
}
```

---

## 8. 增量更新与流式渲染

### 8.1 JSONL 流式解析

A2UI 协议使用 JSONL（每行一个完整 JSON 对象）支持流式传输。jaether 支持逐行解析：

```cpp
class JA2UIParser {
public:
    // 逐行处理 JSONL
    void parseJSONL(const std::string& jsonl) {
        std::istringstream stream(jsonl);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            processMessage(line);
        }
    }

    // 处理单条消息
    void processMessage(const std::string& messageJson) {
        auto value = JJSONParser::parse(messageJson);
        auto& obj = std::get<JJSONObject>(value.value);
        
        if (obj.count("createSurface")) {
            handleCreateSurface(obj["createSurface"]);
        } else if (obj.count("updateComponents")) {
            handleUpdateComponents(obj["updateComponents"]);
        } else if (obj.count("updateDataModel")) {
            handleUpdateDataModel(obj["updateDataModel"]);
        } else if (obj.count("deleteSurface")) {
            handleDeleteSurface(obj["deleteSurface"]);
        }
    }
};
```

### 8.2 渐进式渲染策略

```
时间线       服务端 (Agent/LLM)                    客户端 (jaether)
──────────────────────────────────────────────────────────────────
  t1    →  {"createSurface": {"surfaceId":"main",...}}
                                                   创建 Surface，分配资源
  t2    →  {"updateComponents": {..., [Column+Text]}}
                                                   构建骨架 UI，runFrame()
  t3    →  {"updateComponents": {..., [Button+Text]}}
                                                   增量添加按钮，runFrame()
  t4    →  {"updateDataModel": {"contents":[...]}}
                                                   填充动态数据，重新绑定
  t5    →  {"updateComponents": {..., [更多组件]}}
                                                   继续增量构建
 ...
  tn    →  UI 逐步完整呈现
```

### 8.3 增量更新实现

```cpp
void JA2UIParser::applyComponentUpdate(const std::string& updateJson,
                                        const std::string& surfaceId) {
    auto descs = parseDescriptors(updateJson);
    
    logicLayer_.beginBatch();  // 批量更新
    
    for (auto& desc : descs) {
        auto existing = surfaceManager_->findComponent(surfaceId, desc.id);
        
        if (existing.isValid()) {
            // 已有组件 → 更新属性
            updateExistingComponent(existing, desc);
        } else {
            // 新组件 → 创建
            JComponentHandle parent = resolveParent(desc, surfaceId);
            auto handle = createComponent(desc, parent);
            surfaceManager_->registerComponent(surfaceId, desc.id, handle);
        }
    }
    
    logicLayer_.endBatch();  // 统一触发布局
}
```

---

## 9. 实现计划

### 阶段一：基础解析器 + 简单控件（2 周）

**目标**：实现 JSON→组件树的单向转换，支持基础控件。

| 任务 | 说明 | 文件 |
|------|------|------|
| `JA2UIParser` 核心实现 | JSON 解析 → ComponentDescriptor → JComponentHandle | `src/A2UIParser.cpp` |
| `JJComponentCatalog` 实现 | 组件类型注册表 + 基础目录 | `src/ComponentCatalog.cpp` |
| `JJSurfaceManager` 实现 | Surface 创建/删除 + ID 映射 | `src/SurfaceManager.cpp` |
| `JComponentType::Card` 扩展 | types.h 和 Direct2DRenderer 新增 Card 支持 | `include/aether/types.h`, `src/Direct2DRenderer.cpp` |
| `JLogicLayer` A2UI 接口 | 新增 loadFromA2UI/streamComponent | `include/aether/LogicLayer.h`, `src/LogicLayer.cpp` |
| 单元测试 | Text, Button, Row, Column, Card 解析测试 | `tests/A2UIParser_test.cpp` |

**交付物**：能通过 JSON 初始化 TodoApp 界面。

### 阶段二：生成器 + 双向转换（1 周）

**目标**：实现组件树→JSON 的逆向转换，保证双向一致性。

| 任务 | 说明 | 文件 |
|------|------|------|
| `JA2UIGenerator` 实现 | 组件树遍历 → A2UI JSON 生成 | `src/A2UIGenerator.cpp` |
| Snapshot 系统集成 | `getComponentTreeA2UI()` 方法 | `src/TestController.cpp` |
| 往返测试 | 解析→生成→解析 幂等性测试 | `tests/A2UIRoundtrip_test.cpp` |

**交付物**：C++ API 创建的界面可导出为 A2UI JSON。

### 阶段三：数据模型 + 增量更新（1.5 周）

**目标**：支持数据绑定和增量组件更新。

| 任务 | 说明 | 文件 |
|------|------|------|
| `JJDataModel` 实现 | 数据模型存储 + 绑定通知 | `src/DataModel.cpp` |
| 增量更新 | `applyComponentUpdate` 支持已有 ID 更新 | `src/A2UIParser.cpp` |
| JSONL 流式解析 | `parseJSONL` 逐行解析 | `src/A2UIParser.cpp` |
| 集成测试 | 完整 A2UI 工作流测试 | `tests/A2UIIntegration_test.cpp` |

**交付物**：支持渐进式 UI 构建和数据驱动渲染。

### 阶段四：v0.8 兼容 + 文档（1 周）

**目标**：兼容 v0.8 协议，完善文档和示例。

| 任务 | 说明 |
|------|------|
| v0.8→v0.9 转换层 | `convertV08ToV09` |
| CheckBox 复合组件 | Container+Button+Text 组合实现 |
| 完整 TodoApp 示例 | 通过 JSON 初始化完整待办应用 |
| 性能测试 | 100+ 组件 JSON 解析性能验证 |

---

## 10. 附录

### 10.1 A2UI 版本差异对照表

| 特性 | v0.8 | v0.9 (jaether 主要支持) |
|------|------|------------------------|
| 创建表面 | `beginRendering` | `createSurface` |
| 组件定义 | `surfaceUpdate` | `updateComponents` |
| 数据模型 | `dataModelUpdate` | `updateDataModel` |
| 组件包装 | `{"Text": {...}}` | `"component": "Text"` |
| 字符串值 | `{"literalString": "hi"}` | `"hi"` |
| 子节点列表 | `{"explicitList": [...]}` | `[...]` |
| 文字样式 | `usageHint` | `variant` |
| 布局对齐 | `distribution` / `alignment` | `justify` / `align` |
| 按钮主要样式 | `primary: true` | `variant: "primary"` |
| 动作格式 | `{"name": "..."}` | `{"event": {"name": "..."}}` |
| 数据模型 key | `key` + `valueString` 等 | 保持一致 |

### 10.2 新增文件清单

| 文件 | 说明 |
|------|------|
| `include/aether/A2UIParser.h` | JA2UIParser 头文件 |
| `include/aether/A2UIGenerator.h` | JA2UIGenerator 头文件 |
| `include/aether/SurfaceManager.h` | JJSurfaceManager 头文件 |
| `include/aether/DataModel.h` | JJDataModel 头文件 |
| `include/aether/ComponentCatalog.h` | JJComponentCatalog 头文件 |
| `src/A2UIParser.cpp` | JA2UIParser 实现 |
| `src/A2UIGenerator.cpp` | JA2UIGenerator 实现 |
| `src/SurfaceManager.cpp` | JJSurfaceManager 实现 |
| `src/DataModel.cpp` | JJDataModel 实现 |
| `src/ComponentCatalog.cpp` | JJComponentCatalog 实现 |
| `tests/A2UIParser_test.cpp` | A2UI 解析测试 |
| `tests/A2UIGenerator_test.cpp` | A2UI 生成测试 |
| `tests/A2UIIntegration_test.cpp` | A2UI 集成测试 |

### 10.3 修改文件清单

| 文件 | 修改内容 |
|------|----------|
| `include/aether/LogicLayer.h` | 新增 A2UI 接口 + 成员 |
| `include/aether/types.h` | JComponentType 新增 Card |
| `src/LogicLayer.cpp` | A2UI 惰性初始化 |
| `src/Direct2DRenderer.cpp` | Card 组件渲染逻辑 |
| `src/TestController.cpp` | 新增 getComponentTreeA2UI() |
| `CMakeLists.txt` | 新增源文件 |

### 10.4 参考文献

- [A2UI Protocol v0.8](https://a2ui.org/specification/v0.8-a2ui/)
- [A2UI Protocol v0.9](https://a2ui.org/specification/v0.9-a2ui/)
- [A2UI Component Gallery](https://a2ui.org/reference/components/)
- [A2UI Message Reference](https://a2ui.org/reference/messages/)
- [Google A2UI GitHub](https://github.com/google/A2UI)

### 10.5 变更记录

| 版本 | 日期 | 变更内容 |
|------|------|----------|
| 1.0.0 | 2026-05-25 | 初始详细设计 |

---

**文档结束**
