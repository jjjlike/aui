# Aether UI Engine 完整设计文档

> **版本**：1.0.0  
> **目标**：设计一个面向AI友好的C++ UI引擎，核心特性为**逻辑与渲染完全分离**的两层架构，支持无头（Headless）运行、确定性状态、AI驱动的测试与生成。  
> **设计原则**：AI友好、高性能、轻量级、跨平台。

---

## 目录

1. [概述](#1-概述)
2. [总体架构](#2-总体架构)
3. [逻辑层详细设计](#3-逻辑层详细设计)
4. [渲染层详细设计](#4-渲染层详细设计)
5. [AI测试支持](#5-ai测试支持)
6. [关键算法说明](#6-关键算法说明)
7. [单元测试设计](#7-单元测试设计)
8. [分阶段实现计划](#8-分阶段实现计划)
9. [风险与缓解](#9-风险与缓解)
10. [附录](#10-附录)

---

## 1. 概述

### 1.1 背景与动机
传统UI框架将界面逻辑与图形渲染紧密耦合，导致自动化测试脆弱、依赖像素比对、难以被AI理解与操作。随着生成式AI与智能测试的兴起，需要一种**AI原生**的UI引擎，其核心要求：
- AI能**直接操控和观测**UI的所有逻辑状态，无需图形界面。
- 渲染层仅为视觉呈现，不包含任何业务逻辑。
- 支持**可重现、可解释**的测试，与AI工具链无缝集成。

### 1.2 设计目标
| 目标 | 指标 |
|------|------|
| 逻辑与渲染彻底分离 | 逻辑层独立运行，不依赖GPU/窗口系统 |
| AI友好 | 暴露结构化API（JSON/RPC），支持事件注入、状态快照、确定性回放 |
| 轻量级 | 核心库体积 <5MB，无外部依赖 |
| 高性能 | 1000组件界面，布局+事件处理 <0.5ms/帧 |
| 跨平台 | 逻辑层纯C++17，渲染层支持Windows（Direct2D），后续扩展 |

### 1.3 核心概念
| 概念 | 描述 |
|------|------|
| **逻辑层** | 不可见的UI数字孪生，包含组件树、布局引擎、状态管理、事件处理器 |
| **渲染层** | 纯可视层，订阅逻辑层状态，执行绘制并将原始输入转发给逻辑层 |
| **组件** | 逻辑层中的基本UI元素，拥有类型、属性、子节点 |
| **布局引擎** | 计算每个组件的位置和尺寸，采用Flexbox子集 + 增量脏标记 |
| **快照** | 逻辑层某一时刻的完整状态描述（JSON/二进制），用于断言、差异对比 |
| **测试代理** | 外部AI测试程序，通过RPC连接逻辑层，注入事件、获取快照 |

---

## 2. 总体架构

### 2.1 架构图
```
┌─────────────────────────────────────────────────────────────┐
│                        Test Agent (AI)                       │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────┐  │
│  │ Event Gen   │    │ Assertions  │    │ Learning/Repair │  │
│  └──────┬──────┘    └──────┬──────┘    └────────┬────────┘  │
└─────────┼──────────────────┼────────────────────┼────────────┘
          │ RPC (gRPC/WebSocket)  │                    │
          ▼                      ▼                    │
┌─────────────────────────────────────────────────────┼────────┐
│                     Logic Layer                      │        │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │        │
│  │ Component    │  │ State        │  │ Event      │ │        │
│  │ Tree (SoA)   │◄─┤ Manager      │─►│ Dispatcher │ │        │
│  └──────┬───────┘  └──────────────┘  └─────┬──────┘ │        │
│         │ Layout Engine (增量)              │        │        │
│         └────────────────────┬─────────────┘        │        │
│                              ▼                      │        │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │        │
│  │ QuadTree     │  │ Snapshot     │  │ Test API   │◄┼────────┘
│  │ (命中测试)   │  │ Serializer   │  │ (ITestController)│
│  └──────────────┘  └──────────────┘  └────────────┘ │
│         │ RPC Server (gRPC)           ┌────────────┐ │
│         └─────────────────────────────►│ IPC Client │─┼──┐
│                                        └────────────┘ │  │
└───────────────────────────────────────────────────────┘  │
                 │ State Diff & Input Events (IPC)         │
                 ▼                                          │
┌─────────────────────────────────────────────────────────┐│
│                    Render Layer (optional)               ││
│  ┌──────────────┐  ┌──────────────┐  ┌────────────────┐││
│  │ State        │  │ Native       │  │ Input Capture  │││
│  │ Subscriber   │─►│ Painter      │◄─│ (Mouse/Keyboard)│││
│  └──────────────┘  └──────────────┘  └────────┬───────┘││
└────────────────────────────────────────────────┼────────┘│
                                                  │ raw events
                                                  └──────────┘
```

### 2.2 模块划分
| 模块 | 职责 | 依赖 |
|------|------|------|
| **Core** | 组件树存储（SoA）、属性系统、句柄管理 | C++ STL |
| **Layout** | 增量布局引擎（Flexbox），脏标记传播 | Core |
| **QuadTree** | 空间索引，加速命中测试 | Core |
| **StateManager** | 状态存储、变更通知、批处理 | Core |
| **EventDispatcher** | 事件路由、回调管理、命中测试 | Core, QuadTree |
| **Snapshot** | 状态序列化/反序列化（JSON/二进制） | Core, nlohmann/json |
| **TestAPI** | 供AI调用的查询/注入接口 | Core, Snapshot, EventDispatcher |
| **RPCServer** | 基于gRPC的远程控制 | TestAPI |
| **RenderSubscriber** | 监听逻辑层状态变化，生成绘制命令 | Core, IPC |
| **NativeRenderer** | Direct2D绘制 | Windows SDK |
| **InputForwarder** | 捕获系统输入，转发给逻辑层 | Windows SDK |

---

## 3. 逻辑层详细设计

### 3.1 组件模型

#### 3.1.1 传统方案的问题
每个组件单独分配（`std::unique_ptr<Component>`）导致节点分散在堆上，遍历时缓存未命中率高。

#### 3.1.2 优化：基于索引的连续存储 + SoA布局

```cpp
// 组件数据存储（SoA风格）
class ComponentStorage {
public:
    struct ComponentEntry {
        ComponentId id;                     // 全局唯一ID
        ComponentType type;                // 枚举：Button, Text, Container...
        Rect layoutResult;                 // 热数据：计算后的位置和尺寸
        int32_t parentIndex;               // 父节点索引，-1表示根
        std::vector<int32_t> childrenIndices; // 子节点索引列表
        PropertyBlock properties;          // 属性块（见3.2）
        uint32_t generation;               // 世代号，用于检测悬空句柄
        bool visible : 1;
        bool enabled : 1;
        // ... 其他标志
    };
    
    std::vector<ComponentEntry> entries;   // 连续存储
    std::vector<int32_t> freeList;         // 空闲索引回收
};

// 组件句柄（轻量级，值传递）
struct ComponentHandle {
    int32_t index;
    uint32_t generation;
    
    bool isValid(const ComponentStorage& storage) const {
        return index >= 0 && index < storage.entries.size() &&
               storage.entries[index].generation == generation;
    }
};
```

**收益**：遍历 `layoutResult` 时缓存命中率极高，布局和命中测试性能提升2~5倍。

### 3.2 属性系统

#### 3.2.1 优化：预定义属性ID + 扁平存储

```cpp
// 编译期属性ID枚举
enum class PropertyId : uint16_t {
    Text = 0,
    Enabled,
    Width, Height,
    BackgroundColor,
    MarginLeft, MarginTop, MarginRight, MarginBottom,
    FlexGrow, FlexShrink,
    // ... 预留扩展
    UserDefined = 1000
};

// 属性值类型（小对象优化）
class PropertyValue {
    std::variant<int, float, bool, std::string, Color, Rect> data;
    // 对于字符串，使用SSO（Small String Optimization）
};

// 每个组件的属性块
struct PropertyBlock {
    std::vector<PropertyValue> values;    // 索引 = PropertyId
    std::bitset<2000> presence;           // 标记哪些属性已设置
};
```

**访问流程**：
```cpp
void setProperty(ComponentHandle h, PropertyId id, PropertyValue value) {
    auto& block = storage.entries[h.index].properties;
    size_t idx = static_cast<size_t>(id);
    if (idx >= block.values.size()) block.values.resize(idx + 1);
    block.values[idx] = std::move(value);
    block.presence.set(idx);
    markDirty(h, id);   // 触发布局脏标记
}
```

**收益**：属性读写速度提升5~10倍，内存占用减少30%。

### 3.3 布局引擎

#### 3.3.1 增量布局 + 脏标记传播

```cpp
class LayoutEngine {
    struct DirtyFlags {
        bool selfDirty : 1;      // 自身尺寸/位置需要重算
        bool childrenDirty : 1;  // 子节点需要重算
    };
    std::vector<DirtyFlags> dirtyFlags;  // 与ComponentEntry对齐
    
public:
    void markDirty(ComponentHandle h, PropertyId changedProp) {
        if (isLayoutAffecting(changedProp)) {
            int32_t idx = h.index;
            while (idx != -1) {
                dirtyFlags[idx].selfDirty = true;
                dirtyFlags[idx].childrenDirty = true;
                idx = storage.entries[idx].parentIndex;
            }
        }
    }
    
    void relayoutIfNeeded() {
        // 收集需要重算的根（通常是从根节点向下）
        std::vector<int32_t> roots;
        for (size_t i = 0; i < storage.entries.size(); ++i) {
            if (dirtyFlags[i].selfDirty && storage.entries[i].parentIndex == -1)
                roots.push_back(i);
        }
        // 对每个根执行布局（深度优先）
        for (int32_t root : roots) {
            computeLayout(root);
        }
        // 清除脏标记
        clearDirtyFlags();
    }
    
private:
    void computeLayout(int32_t nodeIdx);
};
```

#### 3.3.2 Flexbox子集算法（单行/列）

```cpp
void computeLayout(int32_t nodeIdx) {
    auto& node = storage.entries[nodeIdx];
    // 1. 读取属性：width, height, margin, padding, flexGrow等
    // 2. 计算子节点沿主轴和交叉轴的尺寸
    // 3. 分配剩余空间（按flexGrow）
    // 4. 设置每个子节点的layoutResult
    // 5. 递归调用computeLayout(child)（如果childrenDirty）
}
```

**收益**：常见UI操作布局时间从O(n)降为O(log n)~O(1)。

### 3.4 事件处理与命中测试四叉树

#### 3.4.1 四叉树实现

```cpp
class QuadTree {
    struct Node {
        Rect bounds;
        std::vector<ComponentHandle> components;  // 叶子节点存储句柄
        std::unique_ptr<Node> children[4];         // 非叶子节点
        bool isLeaf;
    };
    
    Node root;
    int maxDepth = 8;
    int maxComponentsPerLeaf = 4;
    
public:
    void rebuild(const std::vector<ComponentHandle>& interactiveComponents,
                 std::function<Rect(ComponentHandle)> getBounds) {
        root = Node{Rect::infinite(), {}, nullptr, true};
        for (auto& h : interactiveComponents) {
            insert(h, getBounds(h));
        }
    }
    
    void update(ComponentHandle h, const Rect& newBounds) {
        remove(h);
        insert(h, newBounds);
    }
    
    std::vector<ComponentHandle> query(const Point& p) const {
        std::vector<ComponentHandle> result;
        queryNode(root, p, result);
        return result;
    }
    
private:
    void insert(ComponentHandle h, const Rect& bounds);
    void remove(ComponentHandle h);
    void queryNode(const Node& node, const Point& p, std::vector<ComponentHandle>& out) const;
};
```

#### 3.4.2 集成到事件派发

```cpp
class EventDispatcher {
    QuadTree quadTree;
    
public:
    void onLayoutComplete() {
        // 收集所有可交互组件（非容器、可见、启用）
        std::vector<ComponentHandle> interactive;
        for (auto& entry : storage.entries) {
            if (entry.visible && entry.enabled && 
                entry.type != ComponentType::Container) {
                interactive.push_back(ComponentHandle{idx, entry.generation});
            }
        }
        quadTree.rebuild(interactive, [this](ComponentHandle h) {
            return storage.entries[h.index].layoutResult;
        });
    }
    
    ComponentHandle hitTest(const Point& p) {
        auto candidates = quadTree.query(p);
        for (auto& h : candidates) {
            if (storage.entries[h.index].visible && 
                storage.entries[h.index].enabled &&
                storage.entries[h.index].layoutResult.contains(p)) {
                return h;
            }
        }
        return {};
    }
};
```

**收益**：命中测试从O(n)降为O(log n)，高频鼠标移动场景CPU占用可忽略。

### 3.5 状态管理与批处理

```cpp
class StateManager {
    struct PendingChange {
        ComponentHandle handle;
        PropertyId id;
        PropertyValue value;
    };
    std::vector<PendingChange> batchBuffer;
    bool inBatch = false;
    
public:
    void beginBatch() { inBatch = true; }
    void endBatch() {
        for (auto& change : batchBuffer) {
            applyChange(change);
        }
        batchBuffer.clear();
        inBatch = false;
        layoutEngine.relayoutIfNeeded();  // 统一触发布局
        notifyRenderSubscribers();        // 通知渲染层
    }
    
    void setProperty(ComponentHandle h, PropertyId id, PropertyValue value) {
        if (inBatch) {
            batchBuffer.push_back({h, id, std::move(value)});
        } else {
            applyChange({h, id, std::move(value)});
            layoutEngine.relayoutIfNeeded();
            notifyRenderSubscribers();
        }
    }
};
```

### 3.6 快照系统

支持JSON和二进制两种格式。二进制使用FlatBuffers实现零拷贝访问。

```cpp
class Snapshot {
    std::vector<uint8_t> data;  // 二进制buffer
public:
    static Snapshot capture(const ComponentStorage& storage);
    void apply(ComponentStorage& storage) const;
    std::string toJSON() const;
    static Snapshot fromJSON(const std::string& json);
};
```

### 3.7 测试API（ITestController）

```cpp
class ITestController {
public:
    virtual std::string getComponentTreeJSON() = 0;
    virtual std::string getProperty(const std::string& id, const std::string& prop) = 0;
    virtual bool setProperty(const std::string& id, const std::string& prop, const std::string& valueJson) = 0;
    virtual void injectMouseEvent(const std::string& type, int x, int y, int button) = 0;
    virtual void injectKeyEvent(const std::string& type, int keyCode, int modifiers) = 0;
    virtual void injectTextInput(const std::string& text) = 0;
    virtual void advanceTime(int ms) = 0;
    virtual std::string takeSnapshot() = 0;
    virtual bool waitForCondition(const std::string& jsonPath, const std::string& expectedValue, int timeoutMs) = 0;
    virtual std::vector<std::string> getEventLog() = 0;
};
```

### 3.8 RPC服务（gRPC）

定义`.proto`文件：
```protobuf
service AetherTest {
    rpc GetComponentTree(Empty) returns (JSONString);
    rpc GetProperty(PropertyRequest) returns (JSONValue);
    rpc SetProperty(SetPropertyRequest) returns (Empty);
    rpc InjectEvent(Event) returns (Empty);
    rpc TakeSnapshot(Empty) returns (Snapshot);
    rpc WaitForCondition(Condition) returns (Bool);
    rpc GetEventLog(Empty) returns (EventLog);
}
```

### 3.9 GPU加速扩展预留

布局算法通过策略模式抽象：

```cpp
class ILayoutSolver {
public:
    virtual ~ILayoutSolver() = default;
    virtual void compute(ComponentStorage& storage, 
                         const std::vector<int32_t>& dirtyRoots) = 0;
};

class FlexboxSolver : public ILayoutSolver { /* CPU实现 */ };
// class GpuFlexboxSolver : public ILayoutSolver { /* CUDA实现 */ };
```

测试模式下强制使用CPU求解器，保证确定性。

---

## 4. 渲染层详细设计

### 4.1 职责限定
- 不存储任何业务状态
- 仅做三件事：
  1. 连接逻辑层（IPC），订阅状态变更
  2. 根据组件树和布局信息，调用图形API绘制
  3. 捕获操作系统输入事件，转发给逻辑层

### 4.2 通信协议
- 逻辑层 → 渲染层：初始全量组件树（JSON），后续增量补丁（JSON Patch）
- 渲染层 → 逻辑层：输入事件流（JSON）

### 4.3 Windows实现（Direct2D）
```cpp
class Direct2DRenderer : public IRenderSubscriber {
    void onStateUpdate(const Patch& patch) override {
        // 解析增量，更新绘制缓存
    }
    void paint() override {
        for (auto& cmd : drawCommands) {
            renderTarget->DrawRectangle(cmd.bounds, cmd.brush);
            // ...
        }
    }
};
```

---

## 5. AI测试支持

### 5.1 无头测试工作流
1. AI测试脚本启动逻辑层进程（可独立于渲染层）
2. 通过RPC获取初始组件树JSON
3. 注入事件序列
4. 调用`takeSnapshot()`获取状态
5. 使用断言或AI模型比较快照
6. 失败时获取事件日志和快照差异

### 5.2 确定性保证
- 所有测试API基于纯CPU逻辑层
- 即使引擎配置了GPU求解器，测试时自动切换回CPU
- 支持事件录制与回放

### 5.3 语义断言扩展（可选）
```python
# Python 示例
client = AetherClient("localhost:50051")
client.assert_semantic("the submit button is enabled")  # 内部转换为属性查询
```

---

## 6. 关键算法说明

### 6.1 增量布局算法（伪代码）

```text
Algorithm: Incremental Layout with Dirty Propagation
Input: ComponentStorage storage, DirtyFlags flags
Output: Updated layout results

function relayoutIfNeeded():
    roots = []
    for each component i in storage:
        if flags[i].selfDirty and storage[i].parentIndex == -1:
            roots.append(i)
    for each root in roots:
        computeLayout(root)
    clearDirtyFlags(flags)

function computeLayout(nodeIdx):
    node = storage[nodeIdx]
    if not flags[nodeIdx].selfDirty and not flags[nodeIdx].childrenDirty:
        return  // 干净节点跳过
    
    // 1. 确定主轴方向（水平/垂直）
    // 2. 收集子节点，过滤不可见的
    // 3. 计算每个子节点的基准尺寸（根据width/height或内容）
    // 4. 计算主轴上的总剩余空间 = 容器尺寸 - 子节点基准尺寸之和 - margin
    // 5. 按flexGrow分配剩余空间
    // 6. 计算子节点的最终位置（沿主轴和交叉轴）
    // 7. 设置每个子节点的layoutResult
    // 8. 递归计算子节点内部布局（如果其childrenDirty为true）
    
    flags[nodeIdx].selfDirty = false
    flags[nodeIdx].childrenDirty = false
```

### 6.2 四叉树查询算法

```text
Algorithm: QuadTree Query
Input: QuadTree root node, Point p
Output: List of ComponentHandle candidates

function query(node, p, result):
    if not node.bounds.contains(p):
        return
    if node.isLeaf:
        for each component in node.components:
            if component.bounds.contains(p):
                result.append(component)
    else:
        for each child in node.children:
            query(child, p, result)

function queryPoint(p):
    result = []
    query(root, p, result)
    // 按深度优先或Z-order排序，返回最上层的组件
    return result
```

### 6.3 属性ID映射

```cpp
// 编译期生成ID映射表
constexpr PropertyId getPropertyId(const char* name) {
    // 使用constexpr哈希或字符串比较（编译期）
    if (strcmp(name, "text") == 0) return PropertyId::Text;
    if (strcmp(name, "enabled") == 0) return PropertyId::Enabled;
    // ...
    return PropertyId::UserDefined;
}
```

---

## 7. 单元测试设计

### 7.1 测试框架
使用 **Google Test** + **Google Benchmark**。

### 7.2 模块测试策略

| 模块 | 测试重点 | 自动化工具 |
|------|----------|------------|
| ComponentStorage | 增删改查、句柄有效性、迭代器缓存友好 | GTest |
| PropertySystem | 属性读写性能、ID映射正确性、绑定求值 | GTest + Benchmark |
| LayoutEngine | 增量脏标记、Flexbox计算结果、多级嵌套 | GTest（快照比对） |
| QuadTree | 插入/删除/查询正确性、复杂度验证 | GTest + 随机测试 |
| EventDispatcher | 命中测试准确率、事件冒泡、异步队列 | GTest |
| Snapshot | 序列化/反序列化一致性、二进制与JSON互转 | GTest |
| TestAPI | RPC调用、事件注入、状态查询 | 集成测试（Python客户端） |
| RenderSubscriber | 状态差异流、IPC通信 | 模拟渲染层 |

### 7.3 示例测试用例

#### 测试：增量布局脏标记
```cpp
TEST(LayoutEngine, DirtyPropagation) {
    auto storage = createTestStorage();  // 根窗口 + 按钮
    LayoutEngine engine(storage);
    
    auto btn = storage.getComponentById("btn");
    engine.markDirty(btn, PropertyId::Width);
    
    EXPECT_TRUE(engine.isDirty(storage.getRoot()));
    EXPECT_TRUE(engine.isDirty(btn));
    
    engine.relayoutIfNeeded();
    EXPECT_FALSE(engine.isDirty(storage.getRoot()));
    EXPECT_FALSE(engine.isDirty(btn));
}
```

#### 测试：四叉树命中测试
```cpp
TEST(QuadTree, HitTest) {
    QuadTree tree;
    std::vector<ComponentHandle> comps = makeComponents(1000); // 随机位置
    tree.rebuild(comps, getBounds);
    
    Point p(150, 200);
    auto hits = tree.query(p);
    // 验证命中的组件确实包含该点
    for (auto& h : hits) {
        EXPECT_TRUE(getBounds(h).contains(p));
    }
}
```

#### 性能基准
```cpp
BENCHMARK(LayoutBench, TenThousandComponents) {
    // 创建10000个嵌套组件
    // 修改一个属性，测量布局时间
    // 预期 <1ms
}
```

### 7.4 持续集成
- 每个Pull Request触发所有单元测试和性能基准
- 使用GitHub Actions或Jenkins
- 代码覆盖率要求 >85%

---

## 8. 分阶段实现计划

### 阶段一：核心逻辑层 + 基础优化（5周）

**目标**：实现可运行的逻辑层，支持组件树、属性系统、基础布局、命中测试（四叉树），并提供TestAPI。

**任务列表**：
- [ ] 实现ComponentStorage（SoA + 句柄）
- [ ] 实现PropertySystem（ID化 + 扁平存储）
- [ ] 实现LayoutEngine（Flexbox子集 + 增量脏标记）
- [ ] 实现QuadTree（全量重建 + 增量更新）
- [ ] 实现EventDispatcher（含命中测试）
- [ ] 实现StateManager（批处理）
- [ ] 实现Snapshot（JSON序列化）
- [ ] 实现ITestController基础方法（getTree, getProperty, injectEvent）
- [ ] 编写单元测试覆盖所有模块（覆盖率>80%）
- [ ] 性能基准测试（1000组件，布局<0.5ms，命中测试<0.01ms）

**交付物**：
- 静态库 `aether_logic.lib`
- 头文件 `aether/logic_layer.h`
- 测试报告（包含性能数据）

**自动化测试**：每个模块独立测试，集成测试验证组件树操作流程。

---

### 阶段二：RPC与无头测试集成（2周）

**目标**：暴露gRPC接口，实现远程控制，支持Python测试脚本。

**任务列表**：
- [ ] 定义gRPC proto
- [ ] 实现RPCServer（封装ITestController）
- [ ] 编写Python客户端库（`aether_test`）
- [ ] 实现`waitForCondition`（基于JSONPath轮询）
- [ ] 实现事件录制/回放工具
- [ ] 端到端测试：Python脚本控制C++逻辑层

**交付物**：
- 可独立运行的逻辑层进程（`aether_logic_server.exe`）
- Python包 `aether_test`
- 示例测试脚本

**自动化测试**：Python集成测试，覆盖RPC调用、事件注入、快照比对。

---

### 阶段三：Windows渲染层（3周）

**目标**：实现渲染层，连接逻辑层，形成完整桌面UI应用。

**任务列表**：
- [ ] 实现IPC客户端（命名管道）
- [ ] 实现状态订阅与JSON Patch解析
- [ ] 实现Direct2D绘制器（窗口、按钮、文本、输入框）
- [ ] 实现输入捕获（鼠标、键盘）并转发给逻辑层
- [ ] 整合逻辑层与渲染层（同一进程或分离进程模式）
- [ ] 制作示例应用（计算器、待办列表）

**交付物**：
- 动态库 `aether_render.dll`
- 示例可执行文件

**自动化测试**：模拟渲染层测试（使用mock IPC）。

---

### 阶段四：AI测试增强与优化（2周）

**目标**：提升AI测试易用性，支持语义断言和大规模性能。

**任务列表**：
- [ ] 实现语义断言辅助库（自然语言→属性路径）
- [ ] 优化二进制快照（FlatBuffers）
- [ ] 支持共享内存IPC（高性能模式）
- [ ] 集成AI生成测试用例（使用LLM生成事件序列）
- [ ] 文档和教程编写

**交付物**：
- 语义断言Python扩展
- 性能优化后的逻辑层
- 用户手册

**自动化测试**：回归测试确保优化不破坏功能。

---

### 阶段五：高级特性（可选，按需）

- [ ] GPU加速布局（基于CUDA/DirectCompute）
- [ ] 更多布局算法（Grid、Absolute）
- [ ] 动画系统（基于时间线的属性插值）
- [ ] 主题与样式系统（CSS-like）
- [ ] 其他渲染后端（Skia、WebGPU）

---

## 9. 风险与缓解

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|----------|
| 逻辑层性能不达标 | 中 | 高 | 阶段一严格遵循基准，若不足则换用更高效算法 |
| IPC通信延迟过高 | 中 | 中 | 支持进程内直接调用模式（合并逻辑与渲染） |
| 四叉树维护成本高 | 低 | 中 | 增量更新策略，仅当布局变化时重建 |
| AI测试确定性丢失 | 低 | 高 | 测试路径强制使用CPU求解器，GPU仅用于非测试环境 |
| 跨平台兼容性 | 中 | 中 | 逻辑层纯C++17，渲染层先专注Windows，后期移植 |

---

## 10. 附录

### 10.1 术语表
| 术语 | 解释 |
|------|------|
| SoA | Structure of Arrays，结构体数组，用于提升缓存局部性 |
| Flexbox | CSS弹性盒布局模型 |
| 脏标记 | 标记组件需要重新布局的属性 |
| 四叉树 | 空间划分数据结构，用于快速查询点所在区域 |
| IPC | 进程间通信 |

### 10.2 参考文献
- [Google A2UI 协议草案](https://github.com/google/a2ui)
- [Flutter 渲染管道](https://flutter.dev/docs/resources/architectural-overview)
- [Flexbox 规范](https://www.w3.org/TR/css-flexbox-1/)
- [四叉树实现](https://en.wikipedia.org/wiki/Quadtree)

### 10.3 变更记录
| 版本 | 日期 | 变更内容 |
|------|------|----------|
| 1.0.0 | 2026-05-23 | 初始完整设计 |

---

**文档结束**