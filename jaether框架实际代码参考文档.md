# jaether 框架实际代码参考文档

> **版本**：对应代码 v1.0.0  
> **更新日期**：2026-05-25  
> **定位**：框架师日常参考文档，基于实际代码编写，按代码变动同步更新  
> **命名空间**：`jaether`  
> **类名前缀**：`J`

---

## 目录

1. [基础类型 (types.h)](#1-基础类型-typesh)
2. [属性系统 (property_id.h)](#2-属性系统-property_idh)
3. [组件存储 (ComponentStorage.h/.cpp)](#3-组件存储-componentstorageh)
4. [属性系统实现 (PropertySystem.cpp)](#4-属性系统实现)
5. [布局引擎 (LayoutEngine.h/.cpp)](#5-布局引擎-layoutengineh)
6. [四叉树空间索引 (QuadTree.h/.cpp)](#6-四叉树空间索引-quadtreeh)
7. [事件分发器 (EventDispatcher.h/.cpp)](#7-事件分发器-eventdispatcherh)
8. [状态管理器 (StateManager.h/.cpp)](#8-状态管理器-statemanagerh)
9. [快照系统 (Snapshot.h/.cpp)](#9-快照系统-snapshoth)
10. [测试控制器 (TestController.h/.cpp)](#10-测试控制器-testcontrollerh)
11. [逻辑层 (LogicLayer.h/.cpp)](#11-逻辑层-logiclayerh)
12. [JSON解析器 (JSONParser.h/.cpp)](#12-json解析器-jsonparserh)
13. [A2UI组件目录 (ComponentCatalog.h/.cpp)](#13-a2ui组件目录-componentcatalogh)
14. [Surface管理器 (SurfaceManager.h/.cpp)](#14-surface管理器-surfacemanagerh)
15. [数据模型与绑定 (DataModel.h/.cpp)](#15-数据模型与绑定-datamodelh)
16. [A2UI解析器 (A2UIParser.h/.cpp)](#16-a2ui解析器-a2uiparserh)
17. [A2UI生成器 (A2UIGenerator.h/.cpp)](#17-a2ui生成器-a2uigeneratorh)
18. [Direct2D渲染器 (Direct2DRenderer.h/.cpp)](#18-direct2d渲染器-direct2drendererh)
19. [应用程序壳 (AetherApplication.h/.cpp)](#19-应用程序壳-aetherapplicationh)
20. [日志系统 (Logger.h)](#20-日志系统-loggerh)
21. [语义断言 (SemanticAssertion.h/.cpp)](#21-语义断言)
22. [RPC服务 (RPCServer.h/.cpp, IPCClient.h/.cpp)](#22-rpc服务)
23. [测试覆盖现状](#23-测试覆盖现状)
24. [构建配置 (CMakeLists.txt)](#24-构建配置)
25. [扩展预留区](#25-扩展预留区)

---

## 1. 基础类型 (types.h)

**文件位置**：`include/aether/types.h`

### 1.1 类型别名

```cpp
using JComponentId = uint32_t;    // 组件ID，全局唯一自增
using JGeneration = uint32_t;     // 世代号，检测悬空句柄

constexpr JComponentId INVALID_COMPONENT_ID = 0;
constexpr JGeneration  INVALID_GENERATION  = 0;
```

### 1.2 JPoint — 二维点

```cpp
struct JPoint {
    float x = 0;
    float y = 0;
    bool contains(float px, float py) const;  // 含有点判断
};
```

### 1.3 JSize — 尺寸

```cpp
struct JSize {
    float width = 0;
    float height = 0;
};
```

### 1.4 JRect — 矩形

```cpp
struct JRect {
    float x = 0, y = 0, width = 0, height = 0;
    float left()   const;   // x
    float top()    const;   // y
    float right()  const;   // x + width
    float bottom() const;   // y + height
    bool contains(float px, float py) const;   // 点是否在矩形内
    bool contains(JPoint p) const;
    static JRect infinite();                   // 无限大矩形(-1e6, -1e6, 2e6, 2e6)
    static JRect fromPoints(x1, y1, x2, y2);  // 两点构造
};
```

### 1.5 JColor — 颜色

```cpp
struct JColor {
    uint8_t r = 255, g = 255, b = 255, a = 255;
    JColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255); // 0-255构造
    JColor(float r, float g, float b, float a = 1.0f);        // 0.0-1.0构造
    static JColor fromARGB(uint32_t argb);
    uint32_t toARGB() const;
};
```

### 1.6 JComponentType — 组件类型枚举

```cpp
enum class JComponentType : uint8_t {
    Container,    // 0 — 容器
    Button,       // 1 — 按钮
    Text,         // 2 — 文本
    Input,        // 3 — 输入框
    ScrollView,   // 4 — 滚动视图
    Image,        // 5 — 图片
    Card,         // 6 — 卡片容器（A2UI扩展）
    Custom        // 7 — 自定义
};
```

### 1.7 JComponentHandle — 组件句柄

```cpp
struct JComponentHandle {
    int32_t index = -1;
    JGeneration generation = 0;
    bool isValid() const;           // index >= 0 && generation != 0
    explicit operator bool() const;
    bool operator==(const JComponentHandle&) const;
    bool operator!=(const JComponentHandle&) const;
};
```

### 1.8 Flexbox 枚举

```cpp
enum class JFlexDirection : uint8_t  { Row = 0, Column = 1 };
enum class JFlexWrap : uint8_t       { NoWrap = 0, Wrap = 1 };
enum class JJustifyContent : uint8_t { FlexStart=0, Center=1, FlexEnd=2, SpaceBetween=3, SpaceAround=4 };
enum class JAlignItems : uint8_t     { FlexStart=0, Center=1, FlexEnd=2, Stretch=3 };
enum class JAlignContent : uint8_t   { FlexStart, Center, FlexEnd, Stretch, SpaceBetween, SpaceAround };
enum class JLayoutEngineMode : uint8_t { Normal, Test };
```

### 1.9 JEventType — 事件类型枚举

```cpp
enum class JEventType {
    Click, MouseDown, MouseUp, MouseMove, MouseEnter, MouseLeave,
    KeyDown, KeyUp, TextInput, Focus, Blur, Change
};
```

### 1.10 事件结构体

```cpp
struct JMouseEvent { JEventType type; JPoint position; int button=0; int clickCount=1; };
struct JKeyEvent   { JEventType type; int keyCode=0; int modifiers=0; char text=0; };
struct JEvent      { JEventType type; JComponentHandle target; JComponentHandle currentTarget;
                     JMouseEvent mouse; JKeyEvent key; std::string text; bool handled=false; bool bubbles=true; };
struct JRecordedEvent { int64_t timestamp; JEventType eventType; JMouseEvent mouseEvent; JKeyEvent keyEvent; std::string textData; };
using JEventCallback = std::function<void(JEvent&)>;
```

---

## 2. 属性系统 (property_id.h)

**文件位置**：`include/aether/property_id.h`

### 2.1 JPropertyId 枚举

| 类别 | ID | 枚举名 | jaether名 |
|------|----|--------|----------|
| 基础 | 1 | Text | text |
| 基础 | 2 | Enabled | enabled |
| 基础 | 3 | Visible | visible |
| 布局 | 8-15 | X/Y/Width/Height/MinMax | x/y/width/height/... |
| 外边距 | 20-23 | MarginLeft/Top/Right/Bottom | marginLeft/... |
| 内边距 | 30-33 | PaddingLeft/Top/Right/Bottom | paddingLeft/... |
| Flex | 40-44 | FlexGrow/Shrink/Basis/Direction/Wrap | flexGrow/... |
| 对齐 | 50-53 | JustifyContent/AlignItems/AlignContent/AlignSelf | justifyContent/... |
| 定位 | 60-64 | PositionType/Left/Top/Right/Bottom | positionType/... |
| 样式 | 70-73 | BackgroundColor/BorderColor/Width/Radius | backgroundColor/... |
| 文本 | 80-84 | FontSize/Family/Weight/TextColor/TextAlign | fontSize/... |
| 层级 | 90 | ZIndex | zIndex |
| 自定义 | 1000 | UserDefined | — |

### 2.2 核心API

```cpp
// 编译期常量
constexpr size_t MAX_PROPERTY_ID = 2000;

// 编译期名称→ID映射
constexpr JPropertyId getPropertyIdFromName(std::string_view name);

// ID→名称映射（编译期）
constexpr const char* getPropertyName(JPropertyId id);

// 判断属性是否影响布局（编译期）
constexpr bool isLayoutAffectingProperty(JPropertyId id);
```

### 2.3 JPropertyValue — 属性值

**定义位置**：`include/aether/ComponentStorage.h`

```cpp
struct JPropertyValue {
    using ValueType = std::variant<std::monostate, int, float, bool,
                                    std::string, JColor, JRect,
                                    JFlexDirection, JFlexWrap,
                                    JJustifyContent, JAlignItems, JAlignContent>;
    ValueType value;
    
    bool hasValue() const;              // 是否为monostate
    template<T> T get() const;          // 获取值
    template<T> bool is() const;        // 类型检查
    std::string toString() const;       // 序列化为字符串
    static JPropertyValue fromString(JPropertyId id, const std::string& str);
};
```

### 2.4 JPropertyBlock — 属性块

```cpp
struct JPropertyBlock {
    std::vector<JPropertyValue> values;   // 索引 = PropertyId
    std::bitset<MAX_PROPERTY_ID> presence; // 存在标记
    
    void ensureSize(size_t);
    bool hasProperty(JPropertyId id) const;
    const JPropertyValue* getProperty(JPropertyId id) const;
    void setProperty(JPropertyId id, JPropertyValue value);
    void clearProperty(JPropertyId id);
};
```

---

## 3. 组件存储 (ComponentStorage.h/.cpp)

**文件位置**：`include/aether/ComponentStorage.h`、`src/ComponentStorage.cpp`

### 3.1 JComponentEntry — 组件条目（SoA布局）

```cpp
struct JComponentEntry {
    JComponentId id = INVALID_COMPONENT_ID;   // 全局唯一ID
    JComponentType type = JComponentType::Container;
    JRect layoutResult;                        // 布局计算结果（相对父组件坐标）
    int32_t parentIndex = -1;                  // 父组件索引，-1表示根
    std::vector<int32_t> childrenIndices;      // 子组件索引列表
    JPropertyBlock properties;                 // 属性块
    JGeneration generation = 0;
    bool visible = true;
    bool enabled = true;
    std::string debugName;
};
```

### 3.2 JComponentStorage — 对外接口

```cpp
class JComponentStorage {
public:
    // 组件生命周期
    JComponentHandle createComponent(JComponentType type, JComponentHandle parent = {});
    void destroyComponent(JComponentHandle handle);
    
    // 有效性检查
    bool isValid(JComponentHandle handle) const;
    
    // 组件访问（通过句柄）
    JComponentEntry*       getComponent(JComponentHandle handle);
    const JComponentEntry* getComponent(JComponentHandle handle) const;
    JComponentEntry*       getComponentByIndex(int32_t index);
    const JComponentEntry* getComponentByIndex(int32_t index) const;
    
    // 组件查找
    JComponentHandle findById(JComponentId id) const;
    JComponentHandle getRoot() const;
    
    // 统计
    size_t size() const;
    size_t activeCount() const;
    
    // 遍历
    template<typename Func> void forEach(Func&& func);       // 遍历所有活跃组件
    template<typename Func> void forEach(Func&& func) const;
    
    // 位置计算
    JRect getAbsoluteBounds(int32_t index) const;      // 递归累加父偏移 → 窗口绝对坐标
    JRect getAbsoluteBounds(JComponentHandle) const;
    JRect getRelativeBounds(JComponentHandle) const;   // = layoutResult
    
    // 命中测试
    bool containsPoint(JComponentHandle handle, float x, float y) const;  // getAbsoluteBounds + contains
    bool containsPoint(JComponentHandle handle, JPoint point) const;
    
    void clear();
};
```

### 3.3 内部实现关键点

- 使用 `std::vector<JComponentEntry>` 连续存储，遍历时缓存友好
- 使用 `freeList_`（空闲链表）回收已销毁槽位
- 世代号机制：`destroyComponent` 只清除 id 并递增 generation，不移动内存
- `getAbsoluteBounds` 沿 `parentIndex` 链向上累加，O(depth)

---

## 4. 属性系统实现 (PropertySystem.cpp)

**文件位置**：`src/PropertySystem.cpp`

- `JPropertyValue::toString()` — 将 variant 值序列化为可读字符串
- `JPropertyValue::fromString(id, str)` — 根据属性类型从字符串解析值（支持 int/float/bool/string/枚举）
- 注意：`#include "aether/PropertySystem.cpp"` 未被头文件包含，需单独编译

---

## 5. 布局引擎 (LayoutEngine.h/.cpp)

**文件位置**：`include/aether/LayoutEngine.h`、`src/LayoutEngine.cpp`

### 5.1 类接口

```cpp
class JLayoutEngine {
public:
    explicit JLayoutEngine(JComponentStorage& storage);
    
    // 脏标记传播（向上标记所有祖先）
    void markDirty(JComponentHandle h, JPropertyId changedProp);
    void markDirty(JComponentHandle h);
    
    bool isDirty(JComponentHandle h) const;
    void relayoutIfNeeded();           // 只重新布局脏组件
    void forceRelayout();              // 强制全部重算
    
    void setMode(JLayoutEngineMode mode);
    JLayoutEngineMode getMode() const;
    float getTotalLayoutTime() const;  // 累计布局耗时（微秒）
    void resetMetrics();
};
```

### 5.2 内部实现

- `JDirtyFlags { bool selfDirty, childrenDirty }` — 每个组件一个
- `computeLayout(int32_t nodeIdx)` — Flexbox子集算法：
  1. 读取容器尺寸/内边距
  2. 收集可见子组件、计算flex属性
  3. 按主轴/交叉轴方向计算每个子组件的 `layoutResult.x/y/width/height`
  4. 按 `flexGrow`/`flexShrink` 分配剩余/不足空间
  5. 递归处理子组件内部布局
- `getMargin()`/`getPadding()` 等辅助函数

### 5.3 关键修改（A2UI适配）

根组件(`parentIndex==-1`)在每次布局时从属性重新读取Width/Height，支持运行时窗口尺寸调整：

```cpp
bool isRoot = (node.parentIndex == -1);
if (isRoot || node.layoutResult.width <= 0.0f) { ... 从属性读取 ... }
```

---

## 6. 四叉树空间索引 (QuadTree.h/.cpp)

**文件位置**：`include/aether/QuadTree.h`、`src/QuadTree.cpp`

### 6.1 类接口

```cpp
class JQuadTree {
public:
    JQuadTree();                          // 默认无限边界
    explicit JQuadTree(const JRect& bounds);
    
    void rebuild(const std::vector<JComponentHandle>& components,
                 std::function<JRect(JComponentHandle)> getBounds);
    void update(JComponentHandle h, const JRect& newBounds);
    
    std::vector<JComponentHandle> query(const JPoint& p) const;
    std::vector<JComponentHandle> query(const JRect& rect) const;
    
    void clear();
    int getNodeCount() const;
    int getMaxDepth() const;
    
private:
    static constexpr int kMaxDepth = 8;
    static constexpr int kMaxComponentsPerLeaf = 4;
    
    struct Node {
        JRect bounds;
        std::vector<JComponentHandle> components;
        std::unique_ptr<Node> children[4];
        bool isLeaf = true;
    };
    Node root_;
    std::map<std::pair<int,int>, JRect> componentBounds_;  // 句柄→边界缓存
};
```

### 6.2 关键实现

- `JEventDispatcher::onLayoutComplete()` 调用 `rebuild()`，收集所有非Container/可见/启用组件
- 重建时使用 `getAbsoluteBounds()` 获取窗口绝对坐标

---

## 7. 事件分发器 (EventDispatcher.h/.cpp)

**文件位置**：`include/aether/EventDispatcher.h`、`src/EventDispatcher.cpp`

### 7.1 类接口

```cpp
class JEventDispatcher {
public:
    explicit JEventDispatcher(JComponentStorage& storage);
    
    // 布局完成后重建四叉树索引
    void onLayoutComplete();
    
    // 命中测试（使用JQuadTree + getAbsoluteBounds）
    JComponentHandle hitTest(const JPoint& p);
    std::vector<JComponentHandle> hitTestAll(const JPoint& p);
    
    // 事件分发
    void dispatchMouseEvent(const JMouseEvent& event);
    void dispatchKeyEvent(const JKeyEvent& event);
    void dispatchTextInput(const std::string& text);
    
    // 高级输入接口
    void onMouseMove(float x, float y);
    void onMouseDown(float x, float y, int button);
    void onMouseUp(float x, float y, int button);
    void onClick(float x, float y, int button);
    void onKeyDown(int keyCode);
    void onKeyUp(int keyCode);
    void onTextInput(const std::string& text);
    
    // 事件回调注册
    void setMouseCallback(JEventCallback callback);
    void setKeyCallback(JEventCallback callback);
    
    // 事件日志（用于测试/录制）
    const std::vector<JEvent>& getEventLog() const;
    void clearEventLog();
    
    // 录制/回放
    void startRecording(const std::string& sessionId);
    std::string stopRecording();
    bool isRecording() const;
    void playEvents(const std::vector<JRecordedEvent>& events);
    
    float getLastMouseX() const;
    float getLastMouseY() const;
    void setCurrentTime(int64_t time);
    int64_t getCurrentTime() const;
};
```

---

## 8. 状态管理器 (StateManager.h/.cpp)

**文件位置**：`include/aether/StateManager.h`、`src/StateManager.cpp`

### 8.1 JStateObserver — 观察者接口

```cpp
class JStateObserver {
public:
    virtual void onPropertyChanged(JComponentHandle h, JPropertyId id, const JPropertyValue& value) = 0;
    virtual void onComponentCreated(JComponentHandle h) = 0;
    virtual void onComponentDestroyed(JComponentHandle h) = 0;
    virtual void onLayoutComplete() = 0;
};
```

### 8.2 JStateManager — 核心协调组件

```cpp
class JStateManager {
public:
    explicit JStateManager(JComponentStorage& storage);
    
    JComponentHandle createComponent(JComponentType type, JComponentHandle parent = {});
    void destroyComponent(JComponentHandle handle);
    
    void setProperty(JComponentHandle h, JPropertyId id, JPropertyValue value);
    const JPropertyValue* getProperty(JComponentHandle h, JPropertyId id) const;
    
    // 批量更新
    void beginBatch();
    void endBatch();         // 统一应用变更 + relayoutIfNeeded
    bool isInBatch() const;
    
    void setLayoutEngine(JLayoutEngine* engine);
    void setEventDispatcher(JEventDispatcher* dispatcher);
    
    // 观察者管理
    void addObserver(JStateObserver* observer);
    void removeObserver(JStateObserver* observer);
    
    // 通知方法（内部由createComponent/setProperty等调用）
    void notifyPropertyChanged(...);
    void notifyComponentCreated(...);
    void notifyComponentDestroyed(...);
    void notifyLayoutComplete();
    
    JComponentStorage& getStorage();
    const JComponentStorage& getStorage() const;
};
```

---

## 9. 快照系统 (Snapshot.h/.cpp)

**文件位置**：`include/aether/Snapshot.h`、`src/Snapshot.cpp`

### 9.1 数据结构

```cpp
struct JSnapshotComponent {
    JComponentId id;
    JComponentType type;
    JRect layout;
    int32_t parentId;
    std::vector<int32_t> childrenIds;
    bool visible;
    bool enabled;
    std::string debugName;
};

struct JSnapshot {
    std::vector<JSnapshotComponent> components;
    uint64_t timestamp = 0;
    int componentCount = 0;
    std::string version;
};
```

### 9.2 JSnapshotSerializer — 序列化/反序列化

```cpp
class JSnapshotSerializer {
public:
    static JSnapshot capture(const JComponentStorage& storage);
    static void apply(const JSnapshot& snapshot, JComponentStorage& storage);
    
    static std::string toJSON(const JSnapshot& snapshot);           // JSON序列化
    static JSnapshot fromJSON(const std::string& json);             // JSON反序列化
    static std::vector<uint8_t> toBinary(const JSnapshot& snapshot); // 二进制
    static JSnapshot fromBinary(const std::vector<uint8_t>& data);
    
    static bool compare(const JSnapshot& a, const JSnapshot& b);   // 快照比较
    static std::string diff(const JSnapshot& a, const JSnapshot& b); // 差异报告
};
```

---

## 10. 测试控制器 (TestController.h/.cpp)

**文件位置**：`include/aether/TestController.h`、`src/TestController.cpp`

### 10.1 ITestController — 测试接口

```cpp
class ITestController {
public:
    virtual std::string getComponentTreeJSON() = 0;
    virtual std::string getComponentTreeA2UI() = 0;       // 新增：A2UI格式
    virtual std::string getProperty(const std::string& id, const std::string& prop) = 0;
    virtual bool setProperty(const std::string& id, const std::string& prop, const std::string& valueJson) = 0;
    virtual void injectMouseEvent(const std::string& type, int x, int y, int button) = 0;
    virtual void injectKeyEvent(const std::string& type, int keyCode, int modifiers) = 0;
    virtual void injectTextInput(const std::string& text) = 0;
    virtual void advanceTime(int ms) = 0;
    virtual std::string takeSnapshot() = 0;
    virtual bool waitForCondition(const std::string& jsonPath, const std::string& expectedValue, int timeoutMs) = 0;
    virtual std::vector<std::string> getEventLog() = 0;
    virtual JComponentHandle getComponentById(const std::string& id) = 0;
    virtual std::string startRecording() = 0;
    virtual std::string stopRecording() = 0;
    virtual void playback(const std::string& sessionData) = 0;
};
```

### 10.2 JTestController — 实现类

```cpp
class JTestController : public ITestController {
public:
    explicit JTestController(JStateManager& stateManager, JEventDispatcher& dispatcher);
    
    // 所有 ITestController 方法实现
    // ...
    
    // A2UI生成器引用（可选，由LogicLayer在ensureA2UIInitialized时注入）
    void setA2UIGenerator(JA2UIGenerator* generator);
    
    void setCurrentTime(int64_t time);
    int64_t getCurrentTime() const;
    
private:
    JComponentHandle findComponentByIdString(const std::string& id);
    std::string evaluateJSONPath(const std::string& jsonPath);
    std::string generateSessionId();
    
    JStateManager& stateManager_;
    JEventDispatcher& dispatcher_;
    JA2UIGenerator* a2uiGenerator_ = nullptr;
    int64_t currentTime_ = 0;
    std::chrono::steady_clock::time_point startTime_;
};
```

---

## 11. 逻辑层 (LogicLayer.h/.cpp)

**文件位置**：`include/aether/LogicLayer.h`、`src/LogicLayer.cpp`

### 11.1 现有核心接口

```cpp
class JLogicLayer {
public:
    JLogicLayer();
    ~JLogicLayer();
    
    // ----- 子模块访问 -----
    JComponentStorage&       getStorage();
    const JComponentStorage& getStorage() const;
    JLayoutEngine&           getLayoutEngine();
    const JLayoutEngine&     getLayoutEngine() const;
    JEventDispatcher&        getEventDispatcher();
    const JEventDispatcher&  getEventDispatcher() const;
    JStateManager&           getStateManager();
    const JStateManager&     getStateManager() const;
    JTestController&         getTestController();
    const JTestController&   getTestController() const;
    
    // ----- 组件操作（委托给StateManager）-----
    JComponentHandle createComponent(JComponentType type, JComponentHandle parent = {});
    void destroyComponent(JComponentHandle handle);
    void setProperty(JComponentHandle h, JPropertyId id, JPropertyValue value);
    const JPropertyValue* getProperty(JComponentHandle h, JPropertyId id) const;
    void beginBatch();
    void endBatch();
    
    // ----- 测试 -----
    std::string takeSnapshot();
    
    // ----- 布局 -----
    void setMode(JLayoutEngineMode mode);
    void runFrame();  // relayoutIfNeeded + onLayoutComplete
    
    // ----- 事件分发 -----
    void dispatchMouseMove(float x, float y);
    void dispatchMouseDown(float x, float y, int button);
    void dispatchMouseUp(float x, float y, int button);
    void dispatchClick(float x, float y, int button);
    void dispatchKeyDown(int keyCode);
    void dispatchKeyUp(int keyCode);
    void dispatchTextInput(const std::string& text);
    
    // ----- A2UI JSON界面描述接口 (新增) -----
    std::string loadFromA2UI(const std::string& json, const std::string& surfaceId = "main");
    std::string exportToA2UI(const std::string& surfaceId = "main") const;
    void streamComponent(const std::string& json, const std::string& surfaceId = "main");
    JA2UIParser& getA2UIParser();
    JA2UIGenerator& getA2UIGenerator();
    JJSurfaceManager& getSurfaceManager();
    JJDataModel& getDataModel();
    
private:
    void ensureA2UIInitialized();  // 惰性初始化所有A2UI模块
    
    JComponentStorage storage_;
    JLayoutEngine layoutEngine_{storage_};
    JEventDispatcher eventDispatcher_{storage_};
    JStateManager stateManager_{storage_};
    JTestController testController_{stateManager_, eventDispatcher_};
    
    std::unique_ptr<JA2UIParser> a2uiParser_;
    std::unique_ptr<JA2UIGenerator> a2uiGenerator_;
    std::unique_ptr<JJSurfaceManager> surfaceManager_;
    std::unique_ptr<JJDataModel> dataModel_;
};
```

### 11.2 关键实现流程

```
JLogicLayer构造函数:
  → 创建所有子模块
  → stateManager_ 持有 storage_ 引用
  → 设置 LayoutEngine/EventDispatcher 到 StateManager

runFrame():
  1. layoutEngine_.relayoutIfNeeded()
  2. eventDispatcher_.onLayoutComplete()  // 重建四叉树

loadFromA2UI(json, surfaceId):
  1. ensureA2UIInitialized()  // 惰性创建 A2UI 模块
  2. a2uiParser_->parseAndApply(json, surfaceId)

ensureA2UIInitialized():
  1. 创建 JA2UIParser（内含 ComponentCatalog/SurfaceManager/DataModel）
  2. 创建 JA2UIGenerator
  3. testController_.setA2UIGenerator(a2uiGenerator_.get())
```

---

## 12. JSON解析器 (JSONParser.h/.cpp)

**文件位置**：`include/aether/JSONParser.h`、`src/JSONParser.cpp`

### 12.1 通用JSON类型

```cpp
using JJSONObject = std::map<std::string, JJSONValue>;
using JJSONArray  = std::vector<JJSONValue>;

struct JJSONValue {
    using ValueType = std::variant<int, float, bool, std::string,
                                    std::nullptr_t, JJSONArray, JJSONObject>;
    ValueType value;
    JJSONValue() : value(nullptr) {}
    template<typename T> JJSONValue(T v) : value(std::move(v)) {}
};
```

### 12.2 JJSONParser — 解析/序列化

```cpp
class JJSONParser {
public:
    static JJSONValue parse(const std::string& json);
    static std::string stringify(const JJSONValue& value);
};
```

### 12.3 JJSONPatch — (骨架，待完善)

```cpp
class JJSONPatch {
public:
    JJSONPatch();
    ~JJSONPatch();
    bool parse(const std::string& json);
    void apply(JJSONObject& root) const;
    std::string toString() const;
};
```

---

## 13. A2UI组件目录 (ComponentCatalog.h/.cpp)

**文件位置**：`include/aether/ComponentCatalog.h`、`src/ComponentCatalog.cpp`

### 13.1 内置组件类型

| A2UI名 | jaether类型 | 布局? | 必填属性 | 可选属性 |
|--------|-----------|-------|---------|---------|
| Row | Container | ✅ | — | children, justify, align, distribution, alignment, weight |
| Column | Container | ✅ | — | children, justify, align, distribution, alignment, weight |
| Text | Text | ❌ | text | variant, usageHint, weight, width, height |
| Button | Button | ❌ | — | child, action, variant, primary, weight, width, height, text |
| TextField | Input | ❌ | — | label, value, text, textFieldType, weight, width, height |
| Card | Container | ❌ | — | child, weight, width, height |
| CheckBox | Button | ❌ | label | value, weight, width, height |

### 13.2 类接口

```cpp
class JJComponentCatalog {
public:
    struct ComponentTypeInfo {
        std::string a2uiName;
        JComponentType jaetherType;
        bool isLayout;
        std::vector<std::string> requiredProps;
        std::vector<std::string> optionalProps;
    };
    
    JJComponentCatalog();  // 注册所有内置类型
    void registerType(const ComponentTypeInfo& info);  // 支持覆盖
    
    JComponentType getType(const std::string& a2uiName) const;     // 正查 → Custom
    std::string getA2UIName(JComponentType type) const;            // 反查 → ""
    bool isValidProperty(const std::string& a2uiName, const std::string& propName) const;
    bool isRegistered(const std::string& a2uiName) const;
    JPropertyId getPropertyId(const std::string& a2uiName, const std::string& propName) const;
    std::vector<std::string> getRequiredProps(const std::string& a2uiName) const;
    std::vector<std::string> getLayoutTypes() const;
    bool isLayoutType(const std::string& a2uiName) const;
};
```

---

## 14. Surface管理器 (SurfaceManager.h/.cpp)

**文件位置**：`include/aether/SurfaceManager.h`、`src/SurfaceManager.cpp`

### 14.1 类接口

```cpp
class JJSurfaceManager {
public:
    struct Surface {
        std::string id;
        std::string catalogId;
        JComponentHandle rootHandle;
        std::unordered_map<std::string, JComponentHandle> a2uiToHandle;   // A2UI ID → handle
        std::unordered_map<int32_t, std::string> handleToA2UI;            // handle.index → A2UI ID
    };
    
    explicit JJSurfaceManager(JComponentStorage& storage);
    
    bool createSurface(const std::string& surfaceId, const std::string& catalogId = "jaether-basic");
    void deleteSurface(const std::string& surfaceId);
    
    void registerComponent(const std::string& surfaceId, const std::string& a2uiId, JComponentHandle handle);
    void unregisterComponent(const std::string& surfaceId, const std::string& a2uiId);
    
    JComponentHandle findComponent(const std::string& surfaceId, const std::string& a2uiId) const;
    std::string findA2UIId(const std::string& surfaceId, JComponentHandle handle) const;
    
    JComponentHandle getRootComponent(const std::string& surfaceId) const;
    void setRootComponent(const std::string& surfaceId, JComponentHandle handle);
    
    std::vector<std::string> getAllComponentIds(const std::string& surfaceId) const;
    std::vector<JComponentHandle> getAllComponentHandles(const std::string& surfaceId) const;
    bool hasSurface(const std::string& surfaceId) const;
    std::string getCatalogId(const std::string& surfaceId) const;
    std::vector<std::string> getAllSurfaceIds() const;
    size_t getComponentCount(const std::string& surfaceId) const;
    
private:
    JComponentStorage& storage_;
    std::unordered_map<std::string, Surface> surfaces_;
};
```

---

## 15. 数据模型与绑定 (DataModel.h/.cpp)

**文件位置**：`include/aether/DataModel.h`、`src/DataModel.cpp`

### 15.1 类接口

```cpp
class JJDataModel {
public:
    using BindingCallback = std::function<void(JComponentHandle, JPropertyId, const JJSONValue&)>;
    
    JJDataModel();
    void setBindingCallback(BindingCallback callback);
    
    // A2UI数据模型更新 (key-value数组格式)
    void applyUpdate(const std::string& surfaceId, const std::string& path, const JJSONArray& contents);
    
    // 直接路径读写
    JJSONValue getValue(const std::string& surfaceId, const std::string& path) const;
    void setValue(const std::string& surfaceId, const std::string& path, const JJSONValue& value);
    
    // 属性绑定
    void bindProperty(const std::string& surfaceId, JComponentHandle handle, JPropertyId propId, const std::string& dataPath);
    void unbindProperty(const std::string& surfaceId, JComponentHandle handle, const std::string& dataPath);
    void notifyBindings(const std::string& surfaceId, const std::string& changedPath);
    
    // 导出/管理
    std::string exportJSON(const std::string& surfaceId) const;
    bool hasDataModel(const std::string& surfaceId) const;
    void clearDataModel(const std::string& surfaceId);
    bool pathExists(const std::string& surfaceId, const std::string& path) const;
};
```

### 15.2 实现要点

- 数据模型存储为 `std::unordered_map<string, JJSONObject>` — 每个surface一个
- `setValueAtPath` 自动创建不存在的中间节点
- `notifyBindings` 在 setValue/applyUpdate 后自动调用绑定的回调
- `applyUpdate` 支持 `valueString`/`valueNumber`/`valueBoolean`/`valueMap` 四种值类型

---

## 16. A2UI解析器 (A2UIParser.h/.cpp)

**文件位置**：`include/aether/A2UIParser.h`、`src/A2UIParser.cpp`

### 16.1 类接口

```cpp
class JA2UIParser {
public:
    explicit JA2UIParser(JLogicLayer& logicLayer);
    
    // 主入口
    std::string parseAndApply(const std::string& json, const std::string& surfaceId);
    
    // 增量更新
    void applyComponentUpdate(const std::string& updateJson, const std::string& surfaceId);
    void applyDataModelUpdate(const std::string& dataJson, const std::string& surfaceId);
    
    // JSONL流式解析
    void parseJSONL(const std::string& jsonl);
    
    // 子模块访问
    JJComponentCatalog& getCatalog();
    JJSurfaceManager& getSurfaceManager();
    JJDataModel& getDataModel();
};
```

### 16.2 内部处理流程

```
parseAndApply:
  1. JJSONParser::parse(json)
  2. 消息类型识别:
     updateComponents    → handleUpdateComponents (v0.9)
     surfaceUpdate       → handleV08→convertV08BoundValue→buildComponentTree (v0.8)
     updateDataModel     → handleUpdateDataModel
     dataModelUpdate     → handleUpdateDataModel (v0.8)
     createSurface       → handleCreateSurface
  3. parseDescriptor/parseV08Component → ComponentDescriptor 结构

buildComponentTree:
  1. 建立 descMap (id→描述符)
  2. 识别根组件（不被任何 children/child 引用的）
  3. 递归创建组件树（优先使用已有组件做增量更新）
  4. 建立父子关系（修改 parentIndex 和 childrenIndices）
  5. logicLayer_.runFrame()

applyProperty:  // A2UI属性→jaether属性映射
  数据绑定: {"path":"/data"} → DataModel::bindProperty
  text       → JPropertyId::Text
  width/height → JPropertyId::Width/Height
  weight     → JPropertyId::FlexGrow
  variant    → JPropertyId::FontSize（h1→32, h2→28, h3→24, h4→20, h5→16, body→14, caption→12）
  justify    → JPropertyId::JJustifyContent（字符串→枚举int）
  align      → JPropertyId::JAlignItems（字符串→枚举int）
  label/value → JPropertyId::Text
  primary    → JPropertyId::BackgroundColor
```

### 16.3 v0.8→v0.9转换映射

| v0.8 | v0.9 |
|------|------|
| `{"Text": {...}}` | `"component": "Text"` |
| `{"literalString": "hi"}` | `"hi"` |
| `{"explicitList": [...]}` | `[...]` |
| `usageHint` | `variant` |
| `distribution` | `justify` |
| `alignment` | `align` |

---

## 17. A2UI生成器 (A2UIGenerator.h/.cpp)

**文件位置**：`include/aether/A2UIGenerator.h`、`src/A2UIGenerator.cpp`

### 17.1 类接口

```cpp
class JA2UIGenerator {
public:
    explicit JA2UIGenerator(const JLogicLayer& logicLayer, const JJSurfaceManager& surfaceManager);
    
    std::string generateSurfaceJSON(const std::string& surfaceId) const;   // updateComponents消息
    JJSONObject generateComponentJSON(JComponentHandle handle, const std::string& a2uiId) const;
    std::string generateDataModelJSON(const std::string& surfaceId) const;  // 桩实现
    std::string getComponentTreeA2UI() const;  // 所有surfaces的JSON
};
```

### 17.2 生成内容

每个组件的JSON导出包含：`id`, `component`, `children`/`child`, `text`, `width`, `height`, `weight`, `variant`, `justify`, `align`

---

## 18. Direct2D渲染器 (Direct2DRenderer.h/.cpp)

**文件位置**：`include/aether/Direct2DRenderer.h`、`src/Direct2DRenderer.cpp`

### 18.1 类接口

```cpp
class JDirect2DRenderer {
public:
    JDirect2DRenderer();
    ~JDirect2DRenderer();
    
    bool initialize(HWND hwnd);
    void shutdown();
    
    void beginDraw();
    void endDraw();
    void resize(int width, int height);
    void clear(const JColor& color);
    
    // 基本绘制
    void drawRect(const JRect& rect, const JColor& color, float strokeWidth = 1.0f);
    void fillRect(const JRect& rect, const JColor& color);
    void drawRoundedRect(const JRect& rect, float rx, float ry, const JColor& color, float strokeWidth = 1.0f);
    void fillRoundedRect(const JRect& rect, float rx, float ry, const JColor& color);
    void drawLine(const JPoint& p1, const JPoint& p2, const JColor& color, float strokeWidth = 1.0f);
    void drawText(const std::string& text, const JRect& rect, const JColor& color,
                  float fontSize = 16.0f, const std::string& fontName = "Segoe UI");
    
    ID2D1HwndRenderTarget* getRenderTarget();
    
    // DPI感知
    float getDpiX() const;   // 当前DPI X (默认96)
    float getDpiY() const;   // 当前DPI Y (默认96)
};
```

### 18.2 组件渲染逻辑 (在应用层)

| JComponentType | 渲染方式 |
|---------------|---------|
| Container | `fillRect(0.95灰)` |
| Button | `fillRoundedRect(红/蓝) + drawText(白)` |
| Text | `drawText(黑)` |
| Input | `fillRect(白) + drawRect(灰边框) + drawText(黑)` |
| Card | `fillRoundedRect(白) + drawRoundedRect(灰边框)` |

---

## 19. 应用程序壳 (AetherApplication.h/.cpp)

**文件位置**：`include/aether/AetherApplication.h`、`src/AetherApplication.cpp`

### 19.1 类接口

```cpp
class JAetherApplication {
public:
    JAetherApplication();
    ~JAetherApplication();
    
    bool initialize(HINSTANCE hInstance, int nCmdShow);
    void shutdown();
    void run();  // 消息循环
    
private:
    // 窗口过程 (静态)
    static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
    
    // 回调
    void onPaint();
    void onResize(int width, int height);
    void onMouseMove(int x, int y);     // 含 DPI 转换: pixels * (96/dpi)
    void onMouseDown(int x, int y, int button);
    void onMouseUp(int x, int y, int button);
    void dispatchClick(int x, int y, int button);
    void onKeyDown(int keyCode);
    void onKeyUp(int keyCode);
    void onChar(char ch);
    void render();                      // 调用 Direct2D 绘制所有组件
    void update();                      // 定时器回调：runFrame + InvalidateRect
    
    void createTestUI();                // 创建示例UI（按钮+文本）
    
    HWND hwnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;
    bool running_ = false;
    std::unique_ptr<JLogicLayer> logicLayer_;
    std::unique_ptr<JDirect2DRenderer> renderer_;
};
```

### 19.2 DPI缩放处理

鼠标坐标从WM消息的物理像素转换为DIP：
```cpp
float dpiScaleX = 96.0f / renderer_->getDpiX();
float dpiScaleY = 96.0f / renderer_->getDpiY();
// 坐标 × dpiScale → DIP坐标，与getAbsoluteBounds一致
```

---

## 20. 日志系统 (Logger.h)

**文件位置**：`include/aether/Logger.h`

### 20.1 类接口

```cpp
enum class JLogLevel { Debug, Info, Warning, Error };

class JLogger {
public:
    static JLogger& getInstance();   // 单例
    
    void setLevel(JLogLevel level);
    void enableFileOutput(bool enabled, const std::string& filename = "aether.log");
    
    void debug(const std::string& message, const char* file = "", int line = 0);
    void info(const std::string& message, const char* file = "", int line = 0);
    void warning(const std::string& message, const char* file = "", int line = 0);
    void error(const std::string& message, const char* file = "", int line = 0);
    
    // 结构化日志（参数化输出）
    void logComponentCreate(JComponentHandle handle, JComponentType type, JComponentHandle parent);
    void logComponentDestroy(JComponentHandle handle);
    void logLayout(JComponentHandle handle, float x, float y, float width, float height);
    void logEvent(const std::string& eventType, float x, float y);
    void logRender(JComponentHandle handle, JComponentType type);
    void logPropertySet(JComponentHandle handle, int propertyId, const std::string& value);
};

// 宏快捷方式
#define AETHER_LOG_DEBUG(msg)    jaether::JLogger::getInstance().debug(msg, __FILE__, __LINE__)
#define AETHER_LOG_INFO(msg)     jaether::JLogger::getInstance().info(msg, __FILE__, __LINE__)
#define AETHER_LOG_WARNING(msg)  jaether::JLogger::getInstance().warning(msg, __FILE__, __LINE__)
#define AETHER_LOG_ERROR(msg)    jaether::JLogger::getInstance().error(msg, __FILE__, __LINE__)
```

---

## 21. 语义断言 (SemanticAssertion.h/.cpp)

**文件位置**：`include/aether/SemanticAssertion.h`、`src/SemanticAssertion.cpp`

骨架预留，用于LLM驱动的语义化UI断言（如"提交按钮已启用"）。

---

## 22. RPC服务 (RPCServer.h/.cpp, IPCClient.h/.cpp)

**文件位置**：`include/aether/RPCServer.h`、`src/RPCServer.cpp`、`include/aether/IPCClient.h`、`src/IPCClient.cpp`

- `JRPCServer` — 创建命名管道服务端，将 JSON 请求转发给 TestController
- `JIPCClient` — 命名管道客户端，连接逻辑层进程

> 当前为基础实现，完整 protobuf/gRPC 集成待后续扩展。

---

## 23. 测试覆盖现状

**测试文件**：17 个测试文件，296 个测试用例，16 个测试套件

### 23.1 测试文件清单

| 测试文件 | 测试模块 | 测试数 |
|---------|---------|--------|
| `ComponentStorage_test.cpp` | 组件存储 | ~21 |
| `PropertySystem_test.cpp` | 属性系统 | ~13 |
| `LayoutEngine_test.cpp` | 布局引擎 | ~25 |
| `QuadTree_test.cpp` | 四叉树 | ~10 |
| `EventDispatcher_test.cpp` | 事件分发 | ~9 |
| `EventRecording_test.cpp` | 事件录制/回放 | ~14 |
| `StateManager_test.cpp` | 状态管理 | ~10 |
| `Snapshot_test.cpp` | 快照系统 | ~15 |
| `TestController_test.cpp` | 测试控制器 | ~8 |
| `LogicLayer_test.cpp` | 逻辑层 | ~5 |
| `JSONParser_test.cpp` | JSON解析 | ~15 |
| `Integration_test.cpp` | 集成测试 | ~6 |
| `ComponentCatalog_test.cpp` | 组件目录 | 22 |
| `SurfaceManager_test.cpp` | Surface管理 | 23 |
| `DataModel_test.cpp` | 数据模型 | 23 |
| `A2UIIntegration_test.cpp` | A2UI解析/生成/往返 | 41 |
| `detailed_logging_test.cpp` | 日志 | ~5 |

### 23.2 运行测试

```bash
cd build-test
cmake -G "Visual Studio 17 2022" -A x64 .. -DBUILD_TESTS=ON
cmake --build . --config Debug --target aether_tests
.\tests\Debug\aether_tests.exe
```

---

## 24. 构建配置 (CMakeLists.txt)

### 24.1 主项目 (CMakeLists.txt)

```cmake
project(Aether VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(AETHER_SOURCES
    src/ComponentStorage.cpp     src/ComponentCatalog.cpp
    src/PropertySystem.cpp       src/SurfaceManager.cpp
    src/LayoutEngine.cpp         src/DataModel.cpp
    src/QuadTree.cpp             src/A2UIParser.cpp
    src/EventDispatcher.cpp      src/A2UIGenerator.cpp
    src/StateManager.cpp
    src/Snapshot.cpp
    src/TestController.cpp
    src/LogicLayer.cpp
    src/IPCClient.cpp            # Win32 only
    src/JSONParser.cpp
    src/Direct2DRenderer.cpp     # Win32 only
    src/AetherApplication.cpp    # Win32 only
    src/RPCServer.cpp           # Win32 only
)

add_library(aether_logic STATIC ${AETHER_SOURCES})
# /W4 /utf-8  链接: ws2_32 d2d1 dwrite

option(BUILD_TESTS ON)     → add_subdirectory(tests)
option(BUILD_SERVER ON)    → aether_logic_server.exe
option(BUILD_APP ON)       → aether_app.exe (WIN32)
```

### 24.2 测试子项目 (tests/CMakeLists.txt)

GoogleTest通过FetchContent自动下载，16个测试源文件编译为 `aether_tests.exe`。

### 24.3 TodoApp子项目 (todo-app/CMakeLists.txt)

独立CMake工程，引用父目录Aether源文件，包含 `todo_app.exe` 和 `TodoAppTest.exe`。

---

## 25. 扩展预留区

> 以下区域供框架师记录计划中的扩展、正在进行的工作和未来方向。

### 25.1 短期计划 (v1.1)

| 项目 | 说明 | 状态 |
|------|------|------|
| JSONPatch完整实现 | `JJSONPatch` 当前为骨架 | 待开始 |
| generateDataModelJSON | `JA2UIGenerator` 当前返回 `"{}"` | 待开始 |
| CheckBox组合组件 | Container+Button+Text 自动创建 | 待开始 |
| 更完善的错误处理 | parseAndApply返回错误详情 | 待开始 |

### 25.2 中期计划 (v1.2)

| 项目 | 说明 | 状态 |
|------|------|------|
| protobuf/gRPC | 替换当前基础RPC实现 | 待开始 |
| GPU加速布局 | CUDA/DirectCompute布局求解器 | 待开始 |
| 动画系统 | 时间线属性插值 | 待开始 |
| 更多渲染后端 | Skia/WebGPU | 待开始 |

### 25.3 变更日志占位

| 日期 | 变更 | 影响模块 |
|------|------|---------|
| | | |

---

**文档结束**
