# Aether UI Engine - 阶段二：RPC与无头测试集成

## 概述

阶段二实现了RPC服务器和Python客户端库，支持远程控制和自动化测试。

## 核心功能

### 1. RPC服务器 (`aether_logic_server.exe`)

基于TCP的简单RPC服务器，支持以下功能：

- **组件树查询**: 获取当前UI组件结构
- **属性操作**: 读取和修改组件属性
- **事件注入**: 模拟鼠标、键盘和文本输入事件
- **快照管理**: 状态快照的获取和对比
- **条件等待**: 等待特定状态条件满足
- **事件录制/回放**: 录制用户操作并回放

### 2. Python客户端库 (`python/aether_test.py`)

```python
from aether_test import AetherClient

client = AetherClient("localhost", 50051)
client.connect()

# 获取组件树
tree = client.get_component_tree()

# 获取属性
width = client.get_property("1", "width")

# 设置属性
client.set_property("1", "width", 900.0)

# 模拟点击
client.click(400, 300)

# 模拟文本输入
client.type_text("Hello World")

# 获取快照
snapshot = client.take_snapshot()

# 等待条件
client.wait_for_condition("root/visible", "true", 5000)

client.disconnect()
```

### 3. 事件录制与回放

```python
from aether_test import AetherClient, EventRecorder

client = AetherClient("localhost", 50051)
client.connect()

recorder = EventRecorder(client)

# 开始录制
recorder.record(duration_ms=5000)

# 保存录制
recorder.save_recording("test_recording.json")

# 回放
recorder.playback()

client.disconnect()
```

## 构建与运行

### 编译RPC服务器

```bash
mkdir build
cd build
cmake .. -DBUILD_SERVER=ON
cmake --build . --config Release
```

### 运行服务器

```bash
# 默认端口 50051
./Release/aether_logic_server.exe

# 指定端口
./Release/aether_logic_server.exe 8080
```

服务器启动后会创建示例UI组件并开始监听连接。

### 运行测试

```bash
cd python
python example_tests.py
```

## 架构设计

### TCP RPC协议

使用简单的文本协议，格式为：

```
METHOD arg1 arg2 arg3...
```

响应为JSON格式。

### 主要方法

| 方法 | 参数 | 说明 |
|------|------|------|
| getComponentTree | 无 | 获取组件树 |
| getProperty | id prop | 获取属性值 |
| setProperty | id prop value | 设置属性值 |
| injectMouseEvent | type x y button | 注入鼠标事件 |
| injectKeyEvent | type keyCode modifiers | 注入键盘事件 |
| injectTextInput | text | 注入文本 |
| takeSnapshot | 无 | 获取快照 |
| waitForCondition | path expected timeout | 等待条件 |
| getEventLog | 无 | 获取事件日志 |
| advanceTime | ms | 推进时间 |

## 文件结构

```
.
├── proto/aether.proto           # gRPC服务定义
├── include/aether/RPCServer.h   # RPC服务器头文件
├── src/RPCServer.cpp            # RPC服务器实现
├── server/main.cpp              # 服务器入口
├── python/
│   ├── aether_test.py          # Python客户端库
│   └── example_tests.py        # 示例测试脚本
└── build3/Release/
    └── aether_logic_server.exe  # 编译好的服务器
```

## 测试示例

### 基本测试

```python
def test_button_click():
    client = AetherClient()
    client.connect()
    
    # 获取按钮位置
    tree = client.get_component_tree()
    button = tree['components'][1]
    
    # 点击按钮
    x = button['layout']['x'] + button['layout']['width'] / 2
    y = button['layout']['y'] + button['layout']['height'] / 2
    client.click(int(x), int(y))
    
    client.disconnect()
```

### 快照对比测试

```python
def test_state_change():
    client = AetherClient()
    client.connect()
    
    # 初始快照
    snapshot1 = client.take_snapshot()
    
    # 触发事件
    client.click(400, 300)
    
    # 新快照
    snapshot2 = client.take_snapshot()
    
    # 比较
    assert snapshot1 != snapshot2, "状态应该改变"
    
    client.disconnect()
```

## 下一阶段

阶段三将实现Windows渲染层，包括：
- Direct2D绘制器
- 窗口和控件渲染
- 输入捕获和转发
- 完整的桌面应用

## 注意事项

1. 服务器需要Windows环境（使用Winsock）
2. Python客户端需要Python 3.6+
3. 默认端口50051可能被占用，可指定其他端口
4. 事件录制功能需要手动触发
