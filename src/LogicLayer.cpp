// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// JLogicLayer.cpp
// 逻辑层模块 - UI引擎的核心逻辑协调层
//
// 功能:
// - 协调组件存储、布局引擎、事件分发器等核心模块
// - 提供统一的API接口
// - 管理每帧的更新流程
// - 提供测试控制接口

#include "aether/LogicLayer.h"
#include "aether/A2UIParser.h"
#include "aether/A2UIGenerator.h"
#include "aether/SurfaceManager.h"
#include "aether/DataModel.h"

namespace jaether {

// 逻辑层构造函数
JLogicLayer::JLogicLayer()
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
JLogicLayer::~JLogicLayer() = default;

// 运行一帧更新
void JLogicLayer::runFrame() {
    // 执行布局更新
    layoutEngine_.relayoutIfNeeded();
    // 更新事件分发器的空间索引
    eventDispatcher_.onLayoutComplete();
}

// 分发鼠标移动事件
// 参数: x, y - 鼠标坐标
void JLogicLayer::dispatchMouseMove(float x, float y) {
    eventDispatcher_.onMouseMove(x, y);
}

// 分发鼠标按下事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JLogicLayer::dispatchMouseDown(float x, float y, int button) {
    eventDispatcher_.onMouseDown(x, y, button);
}

// 分发鼠标释放事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JLogicLayer::dispatchMouseUp(float x, float y, int button) {
    eventDispatcher_.onMouseUp(x, y, button);
}

// 分发鼠标点击事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JLogicLayer::dispatchClick(float x, float y, int button) {
    eventDispatcher_.onClick(x, y, button);
}

// 分发按键按下事件
// 参数: keyCode - 按键码
void JLogicLayer::dispatchKeyDown(int keyCode) {
    eventDispatcher_.onKeyDown(keyCode);
}

// 分发按键释放事件
// 参数: keyCode - 按键码
void JLogicLayer::dispatchKeyUp(int keyCode) {
    eventDispatcher_.onKeyUp(keyCode);
}

// 分发文本输入事件
// 参数: text - 输入的文本
void JLogicLayer::dispatchTextInput(const std::string& text) {
    eventDispatcher_.onTextInput(text);
}

// ========== A2UI JSON界面描述接口实现 ==========

// 确保A2UI模块已初始化
void JLogicLayer::ensureA2UIInitialized() {
    if (!a2uiParser_) {
        a2uiParser_ = std::make_unique<JA2UIParser>(*this);
    }
    if (!a2uiGenerator_) {
        a2uiGenerator_ = std::make_unique<JA2UIGenerator>(*this, a2uiParser_->getSurfaceManager());
    }
    // 将A2UI生成器关联到测试控制器，使getComponentTreeA2UI()可用
    testController_.setA2UIGenerator(a2uiGenerator_.get());
}

// 获取A2UI解析器引用
JA2UIParser& JLogicLayer::getA2UIParser() {
    ensureA2UIInitialized();
    return *a2uiParser_;
}

// 获取A2UI生成器引用
JA2UIGenerator& JLogicLayer::getA2UIGenerator() {
    ensureA2UIInitialized();
    return *a2uiGenerator_;
}

// 获取Surface管理器引用
JJSurfaceManager& JLogicLayer::getSurfaceManager() {
    ensureA2UIInitialized();
    return a2uiParser_->getSurfaceManager();
}

// 获取数据模型引用
JJDataModel& JLogicLayer::getDataModel() {
    ensureA2UIInitialized();
    return a2uiParser_->getDataModel();
}

// 从A2UI JSON加载界面
std::string JLogicLayer::loadFromA2UI(const std::string& json, const std::string& surfaceId) {
    ensureA2UIInitialized();
    return a2uiParser_->parseAndApply(json, surfaceId);
}

// 将当前界面导出为A2UI JSON
std::string JLogicLayer::exportToA2UI(const std::string& surfaceId) const {
    // 由于生成器需要SurfaceManager引用，这里通过解析器间接获取
    // 如果尚未初始化，返回空
    if (!a2uiParser_) {
        return "{}";
    }
    JA2UIGenerator generator(*this, a2uiParser_->getSurfaceManager());
    return generator.generateSurfaceJSON(surfaceId);
}

// 流式添加组件
void JLogicLayer::streamComponent(const std::string& json, const std::string& surfaceId) {
    ensureA2UIInitialized();
    a2uiParser_->parseJSONL(json);
}

} // namespace jaether
