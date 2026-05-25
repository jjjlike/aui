// JTodoApp.cpp
// 待办列表应用实现
// 
// 功能:
// - 添加待办项
// - 标记完成/取消
// - 删除待办项
// - 显示统计信息

#include "TodoApp.h"
#include <aether/Logger.h>
#include <sstream>
#include <iostream>

namespace jaether {

/**
 * 待办应用构造函数
 * @param logicLayer 逻辑层引用
 */
JTodoApp::JTodoApp(JLogicLayer& logicLayer) 
    : logicLayer_(logicLayer) {
}

/**
 * 待办应用析构函数
 */
JTodoApp::~JTodoApp() {
}

/**
 * 初始化应用UI
 * 创建所有UI组件并设置布局
 */
void JTodoApp::initialize() {
    JLogger::getInstance().info("JTodoApp::initialize() 开始初始化UI");
    
    // 创建根容器
    rootContainer_ = logicLayer_.createComponent(JComponentType::Container);
    JLogger::getInstance().info("创建根容器，handle=" + std::to_string(rootContainer_.index));
    logicLayer_.setProperty(rootContainer_, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer_.setProperty(rootContainer_, JPropertyId::Height, JPropertyValue(600.0f));
    logicLayer_.setProperty(rootContainer_, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Column)));
    
    // 创建标题
    title_ = logicLayer_.createComponent(JComponentType::Text, rootContainer_);
    JLogger::getInstance().info("创建标题，handle=" + std::to_string(title_.index));
    logicLayer_.setProperty(title_, JPropertyId::Text, JPropertyValue(std::string("我的待办列表")));
    logicLayer_.setProperty(title_, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer_.setProperty(title_, JPropertyId::Height, JPropertyValue(60.0f));
    logicLayer_.setProperty(title_, JPropertyId::PaddingLeft, JPropertyValue(20.0f));
    logicLayer_.setProperty(title_, JPropertyId::PaddingTop, JPropertyValue(20.0f));
    logicLayer_.setProperty(title_, JPropertyId::FontSize, JPropertyValue(24.0f));
    logicLayer_.setProperty(title_, JPropertyId::FlexShrink, JPropertyValue(1.0f));
    
    // 创建输入区域容器
    inputContainer_ = logicLayer_.createComponent(JComponentType::Container, rootContainer_);
    JLogger::getInstance().info("创建输入区域容器，handle=" + std::to_string(inputContainer_.index));
    logicLayer_.setProperty(inputContainer_, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer_.setProperty(inputContainer_, JPropertyId::Height, JPropertyValue(60.0f));
    logicLayer_.setProperty(inputContainer_, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Row)));
    logicLayer_.setProperty(inputContainer_, JPropertyId::PaddingLeft, JPropertyValue(20.0f));
    logicLayer_.setProperty(inputContainer_, JPropertyId::PaddingRight, JPropertyValue(20.0f));
    logicLayer_.setProperty(inputContainer_, JPropertyId::FlexShrink, JPropertyValue(1.0f));
    
    // 创建输入框
    inputBox_ = logicLayer_.createComponent(JComponentType::Input, inputContainer_);
    JLogger::getInstance().info("创建输入框，handle=" + std::to_string(inputBox_.index));
    logicLayer_.setProperty(inputBox_, JPropertyId::Width, JPropertyValue(400.0f));
    logicLayer_.setProperty(inputBox_, JPropertyId::Height, JPropertyValue(40.0f));
    logicLayer_.setProperty(inputBox_, JPropertyId::PaddingLeft, JPropertyValue(10.0f));
    logicLayer_.setProperty(inputBox_, JPropertyId::PaddingTop, JPropertyValue(10.0f));
    logicLayer_.setProperty(inputBox_, JPropertyId::FlexShrink, JPropertyValue(1.0f));
    
    // 创建添加按钮
    addButton_ = logicLayer_.createComponent(JComponentType::Button, inputContainer_);
    JLogger::getInstance().info("创建添加按钮，handle=" + std::to_string(addButton_.index) + " 父容器=" + std::to_string(inputContainer_.index));
    logicLayer_.setProperty(addButton_, JPropertyId::Text, JPropertyValue(std::string("添加")));
    logicLayer_.setProperty(addButton_, JPropertyId::Width, JPropertyValue(100.0f));
    logicLayer_.setProperty(addButton_, JPropertyId::Height, JPropertyValue(40.0f));
    logicLayer_.setProperty(addButton_, JPropertyId::MarginLeft, JPropertyValue(10.0f));
    
    // 创建待办列表容器
    todoListContainer_ = logicLayer_.createComponent(JComponentType::Container, rootContainer_);
    JLogger::getInstance().info("创建待办列表容器，handle=" + std::to_string(todoListContainer_.index));
    logicLayer_.setProperty(todoListContainer_, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer_.setProperty(todoListContainer_, JPropertyId::Height, JPropertyValue(400.0f));
    logicLayer_.setProperty(todoListContainer_, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Column)));
    logicLayer_.setProperty(todoListContainer_, JPropertyId::PaddingLeft, JPropertyValue(20.0f));
    logicLayer_.setProperty(todoListContainer_, JPropertyId::PaddingRight, JPropertyValue(20.0f));
    logicLayer_.setProperty(todoListContainer_, JPropertyId::FlexShrink, JPropertyValue(1.0f));
    
    // 创建统计信息
    statsText_ = logicLayer_.createComponent(JComponentType::Text, rootContainer_);
    JLogger::getInstance().info("创建统计信息，handle=" + std::to_string(statsText_.index));
    logicLayer_.setProperty(statsText_, JPropertyId::Text, JPropertyValue(std::string("总计: 0 | 已完成: 0 | 未完成: 0")));
    logicLayer_.setProperty(statsText_, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer_.setProperty(statsText_, JPropertyId::Height, JPropertyValue(40.0f));
    logicLayer_.setProperty(statsText_, JPropertyId::PaddingLeft, JPropertyValue(20.0f));
    logicLayer_.setProperty(statsText_, JPropertyId::PaddingTop, JPropertyValue(10.0f));
    logicLayer_.setProperty(statsText_, JPropertyId::FlexShrink, JPropertyValue(1.0f));
    
    // 运行一帧更新
    JLogger::getInstance().info("开始计算布局...");
    logicLayer_.runFrame();
    JLogger::getInstance().info("JTodoApp::initialize() 完成");
}

/**
 * 添加新的待办项
 * @param text 待办文本内容
 */
void JTodoApp::addTodo(const std::string& text) {
    if (text.empty()) {
        return;
    }
    
    // 创建待办项数据
    JTodoItem item;
    item.text = text;
    item.completed = false;
    
    todos_.push_back(item);
    
    // 创建待办项UI
    createTodoItemUI(todos_.size() - 1);
    
    // 更新统计信息
    updateStats();
}

/**
 * 创建单个待办项的UI
 * @param index 待办项索引
 */
void JTodoApp::createTodoItemUI(size_t index) {
    auto& todo = todos_[index];
    
    // 创建待办项容器
    todo.handle = logicLayer_.createComponent(JComponentType::Container, todoListContainer_);
    logicLayer_.setProperty(todo.handle, JPropertyId::Width, JPropertyValue(760.0f));
    logicLayer_.setProperty(todo.handle, JPropertyId::Height, JPropertyValue(50.0f));
    logicLayer_.setProperty(todo.handle, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Row)));
    
    // 创建复选框（用按钮模拟）
    todo.checkbox = logicLayer_.createComponent(JComponentType::Button, todo.handle);
    logicLayer_.setProperty(todo.checkbox, JPropertyId::Text, JPropertyValue(std::string("[ ]")));
    logicLayer_.setProperty(todo.checkbox, JPropertyId::Width, JPropertyValue(40.0f));
    logicLayer_.setProperty(todo.checkbox, JPropertyId::Height, JPropertyValue(40.0f));
    
    // 创建文本组件
    todo.textComp = logicLayer_.createComponent(JComponentType::Text, todo.handle);
    logicLayer_.setProperty(todo.textComp, JPropertyId::Text, JPropertyValue(todo.text));
    logicLayer_.setProperty(todo.textComp, JPropertyId::Width, JPropertyValue(650.0f));
    logicLayer_.setProperty(todo.textComp, JPropertyId::Height, JPropertyValue(40.0f));
    logicLayer_.setProperty(todo.textComp, JPropertyId::PaddingLeft, JPropertyValue(10.0f));
    logicLayer_.setProperty(todo.textComp, JPropertyId::PaddingTop, JPropertyValue(10.0f));
    
    // 创建删除按钮
    todo.deleteBtn = logicLayer_.createComponent(JComponentType::Button, todo.handle);
    logicLayer_.setProperty(todo.deleteBtn, JPropertyId::Text, JPropertyValue(std::string("删除")));
    logicLayer_.setProperty(todo.deleteBtn, JPropertyId::Width, JPropertyValue(60.0f));
    logicLayer_.setProperty(todo.deleteBtn, JPropertyId::Height, JPropertyValue(40.0f));
    logicLayer_.setProperty(todo.deleteBtn, JPropertyId::MarginLeft, JPropertyValue(10.0f));
    
    // 运行一帧更新
    logicLayer_.runFrame();
}

/**
 * 切换待办项的完成状态
 * @param index 待办项索引
 */
void JTodoApp::toggleTodo(size_t index) {
    if (index >= todos_.size()) {
        return;
    }
    
    auto& todo = todos_[index];
    todo.completed = !todo.completed;
    
    // 更新复选框显示
    if (todo.completed) {
        logicLayer_.setProperty(todo.checkbox, JPropertyId::Text, JPropertyValue(std::string("[x]")));
        // 完成时添加删除线样式
        std::string crossedText = std::string("[已完成] ") + todo.text;
        logicLayer_.setProperty(todo.textComp, JPropertyId::Text, JPropertyValue(crossedText));
    } else {
        logicLayer_.setProperty(todo.checkbox, JPropertyId::Text, JPropertyValue(std::string("[ ]")));
        logicLayer_.setProperty(todo.textComp, JPropertyId::Text, JPropertyValue(todo.text));
    }
    
    // 更新统计信息
    updateStats();
}

/**
 * 删除待办项
 * @param index 待办项索引
 */
void JTodoApp::deleteTodo(size_t index) {
    if (index >= todos_.size()) {
        return;
    }
    
    // 销毁UI组件
    auto& todo = todos_[index];
    if (todo.handle.isValid()) {
        logicLayer_.destroyComponent(todo.deleteBtn);
        logicLayer_.destroyComponent(todo.textComp);
        logicLayer_.destroyComponent(todo.checkbox);
        logicLayer_.destroyComponent(todo.handle);
    }
    
    // 从列表中移除
    todos_.erase(todos_.begin() + index);
    
    // 更新统计信息
    updateStats();
}

/**
 * 更新统计信息显示
 */
void JTodoApp::updateStats() {
    std::ostringstream oss;
    oss << "总计: " << getTotalCount() 
        << " | 已完成: " << getCompletedCount()
        << " | 未完成: " << getPendingCount();
    
    logicLayer_.setProperty(statsText_, JPropertyId::Text, JPropertyValue(oss.str()));
}

/**
 * 获取已完成的待办数
 * @return 已完成数
 */
size_t JTodoApp::getCompletedCount() const {
    size_t count = 0;
    for (const auto& todo : todos_) {
        if (todo.completed) {
            ++count;
        }
    }
    return count;
}

/**
 * 检查组件是否在某个位置被点击
 * @param handle 组件句柄
 * @param x 点击x坐标
 * @param y 点击y坐标
 * @return 是否点击了该组件
 */
bool JTodoApp::isComponentClicked(JComponentHandle handle, float x, float y) {
    if (!handle.isValid()) {
        JLogger::getInstance().debug("isComponentClicked - handle无效");
        return false;
    }
    
    // 使用统一的绝对位置命中检测接口
    bool result = logicLayer_.getStorage().containsPoint(handle, x, y);
    
    // 同时获取位置信息用于日志输出
    JRect absoluteBounds = logicLayer_.getStorage().getAbsoluteBounds(handle);
    
    JLogger::getInstance().debug("isComponentClicked - 点击坐标: (" + 
        std::to_string(static_cast<int>(x)) + ", " + 
        std::to_string(static_cast<int>(y)) + "), " +
        "组件绝对位置: (" + 
        std::to_string(static_cast<int>(absoluteBounds.x)) + ", " + 
        std::to_string(static_cast<int>(absoluteBounds.y)) + "), " +
        "组件尺寸: (" + 
        std::to_string(static_cast<int>(absoluteBounds.width)) + "x" + 
        std::to_string(static_cast<int>(absoluteBounds.height)) + "), " +
        "结果: " + std::string(result ? "命中" : "未命中"));
    
    return result;
}

/**
 * 更新每帧的事件处理
 * 检查事件日志并响应
 */
void JTodoApp::update() {
    // 获取事件分发器
    auto& dispatcher = logicLayer_.getEventDispatcher();
    auto& eventLog = dispatcher.getEventLog();
    
    JLogger::getInstance().debug("JTodoApp::update() - 事件数量: " + std::to_string(eventLog.size()));
    
    // 处理事件
    for (const auto& event : eventLog) {
        JLogger::getInstance().debug("JTodoApp::update() - 事件类型: " + std::to_string(static_cast<int>(event.type)));
        
        if (event.type == JEventType::Click) {
            float x = event.mouse.position.x;
            float y = event.mouse.position.y;
            
            JLogger::getInstance().debug("JTodoApp::update() - 点击坐标: (" + 
                std::to_string(static_cast<int>(x)) + ", " + 
                std::to_string(static_cast<int>(y)) + ")");
            JLogger::getInstance().debug("JTodoApp::update() - 添加按钮位置: handle=" + 
                std::to_string(addButton_.index));
            
            // 检查添加按钮
            if (isComponentClicked(addButton_, x, y)) {
                JLogger::getInstance().info("JTodoApp::update() - 检测到添加按钮被点击！");
                addTodo(currentInputText_);
                currentInputText_.clear();
                logicLayer_.setProperty(inputBox_, JPropertyId::Text, JPropertyValue(std::string()));
            } else {
                JLogger::getInstance().debug("JTodoApp::update() - 添加按钮未被点击");
            }
            
            // 检查待办项
            for (size_t i = 0; i < todos_.size(); ++i) {
                // 检查复选框点击
                if (isComponentClicked(todos_[i].checkbox, x, y)) {
                    toggleTodo(i);
                    break;
                }
                // 检查删除按钮点击
                if (isComponentClicked(todos_[i].deleteBtn, x, y)) {
                    deleteTodo(i);
                    break;
                }
            }
        } else if (event.type == JEventType::TextInput) {
            currentInputText_ += event.text;
            logicLayer_.setProperty(inputBox_, JPropertyId::Text, JPropertyValue(currentInputText_));
        }
    }
    
    // 清空事件日志
    dispatcher.clearEventLog();
}

} // namespace jaether
