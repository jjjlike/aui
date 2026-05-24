// LogicLayer.cpp
// 逻辑层模块 - UI引擎的核心逻辑协调层
//
// 功能:
// - 协调组件存储、布局引擎、事件分发器等核心模块
// - 提供统一的API接口
// - 管理每帧的更新流程
// - 提供测试控制接口

#include "aether/LogicLayer.h"

namespace aether {

// 逻辑层构造函数
LogicLayer::LogicLayer()
    : storage_()
    , layoutEngine_(storage_)
    , eventDispatcher_(storage_)
    , stateManager_(storage_)
    , testController_(stateManager_, eventDispatcher_) {
    // 设置状态管理器的依赖
    stateManager_.setLayoutEngine(&layoutEngine_);
    stateManager_.setEventDispatcher(&eventDispatcher_);
}

// 逻辑层析构函数
LogicLayer::~LogicLayer() = default;

// 运行一帧更新
void LogicLayer::runFrame() {
    // 执行布局更新
    layoutEngine_.relayoutIfNeeded();
    // 更新事件分发器的空间索引
    eventDispatcher_.onLayoutComplete();
}

// 分发鼠标移动事件
// 参数: x, y - 鼠标坐标
void LogicLayer::dispatchMouseMove(float x, float y) {
    eventDispatcher_.onMouseMove(x, y);
}

// 分发鼠标按下事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void LogicLayer::dispatchMouseDown(float x, float y, int button) {
    eventDispatcher_.onMouseDown(x, y, button);
}

// 分发鼠标释放事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void LogicLayer::dispatchMouseUp(float x, float y, int button) {
    eventDispatcher_.onMouseUp(x, y, button);
}

// 分发鼠标点击事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void LogicLayer::dispatchClick(float x, float y, int button) {
    eventDispatcher_.onClick(x, y, button);
}

// 分发按键按下事件
// 参数: keyCode - 按键码
void LogicLayer::dispatchKeyDown(int keyCode) {
    eventDispatcher_.onKeyDown(keyCode);
}

// 分发按键释放事件
// 参数: keyCode - 按键码
void LogicLayer::dispatchKeyUp(int keyCode) {
    eventDispatcher_.onKeyUp(keyCode);
}

// 分发文本输入事件
// 参数: text - 输入的文本
void LogicLayer::dispatchTextInput(const std::string& text) {
    eventDispatcher_.onTextInput(text);
}

} // namespace aether
