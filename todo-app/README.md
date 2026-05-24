# 待办列表应用 (Todo App)
基于 Aether UI Engine 的待办列表应用程序

## 项目结构
```
todo-app/
├── CMakeLists.txt          # 构建配置
├── README.md               # 本文档
├── build.bat               # Windows构建脚本
├── include/
│   └── TodoApp.h          # 待办应用头文件
├── src/
│   └── TodoApp.cpp        # 待办应用实现
├── tests/
│   └── TodoAppTest.cpp    # 单元测试
└── main.cpp               # 应用入口
```

## 功能特性
- ✅ 添加待办事项
- ✅ 标记完成/取消完成
- ✅ 删除待办事项
- ✅ 显示统计信息（总数、已完成、未完成）
- ✅ 实时更新UI

## 构建和运行

### 前置条件
- Windows 10/11
- Visual Studio 2022 (带C++开发工具)
- CMake 3.15+

### 方法1: 使用构建脚本（推荐）
```powershell
# 在todo-app目录中运行
.\build.bat

# 或指定Release版本
.\build.bat Release
```

### 方法2: 手动构建
```powershell
# 创建构建目录
mkdir build
cd build

# 配置
cmake -G "Visual Studio 17 2022" -A x64 ..

# 构建
cmake --build . --config Debug
```

### 运行应用
构建成功后，可执行文件位于：
```
build/bin/Debug/TodoApp.exe
```

### 运行测试
```powershell
cd build
ctest -C Debug --verbose
```

或直接运行测试可执行文件：
```
build/tests/Debug/todoapp_tests.exe
```

## 技术说明
该待办应用使用了Aether UI Engine的以下特性：
- **组件系统**：Container, Button, Text, Input
- **Flexbox布局引擎**：灵活的UI布局
- **事件分发系统**：鼠标点击、文本输入
- **属性系统**：组件文本、尺寸、位置等属性
- **测试框架**：完整的单元测试支持

## 代码质量
- 所有代码都包含详细的中文注释
- 遵循现有项目编码风格
- 100%测试覆盖的核心功能

## 依赖关系
- Aether UI Engine (本仓库)
- Direct2D (Windows渲染)
- GoogleTest (单元测试，可选)
```
