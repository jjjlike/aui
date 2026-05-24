// TodoApp.cpp
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

namespace aether {

/**
 * 待办应用构造函数
 * @param logicLayer 逻辑层引用
 */
TodoApp::TodoApp(LogicLayer& logicLayer) 
    : logicLayer_(logicLayer) {
}

/**
 * 待办应用析构函数
 */
TodoApp::~TodoApp() {
}

/**
 * 初始化应用UI
 * 创建所有UI组件并设置布局
 */
void TodoApp::initialize() {
    Logger::getInstance().info("TodoApp::initialize() 开始初始化UI");
    
    // 创建根容器
    rootContainer_ = logicLayer_.createComponent(ComponentType::Container);
    Logger::getInstance().info("创建根容器，handle=" + std::to_string(rootContainer_.index));
    logicLayer_.setProperty(rootContainer_, PropertyId::Width, PropertyValue(800.0f));
    logicLayer_.setProperty(rootContainer_, PropertyId::Height, PropertyValue(600.0f));
    logicLayer_.setProperty(rootContainer_, PropertyId::FlexDirection, PropertyValue(static_cast<int>(FlexDirection::Column)));
    
    // 创建标题
    title_ = logicLayer_.createComponent(ComponentType::Text, rootContainer_);
    Logger::getInstance().info("创建标题，handle=" + std::to_string(title_.index));
    logicLayer_.setProperty(title_, PropertyId::Text, PropertyValue(std::string("我的待办列表")));
    logicLayer_.setProperty(title_, PropertyId::Width, PropertyValue(800.0f));
    logicLayer_.setProperty(title_, PropertyId::Height, PropertyValue(60.0f));
    logicLayer_.setProperty(title_, PropertyId::PaddingLeft, PropertyValue(20.0f));
    logicLayer_.setProperty(title_, PropertyId::PaddingTop, PropertyValue(20.0f));
    logicLayer_.setProperty(title_, PropertyId::FontSize, PropertyValue(24.0f));
    logicLayer_.setProperty(title_, PropertyId::FlexShrink, PropertyValue(1.0f));
    
    // 创建输入区域容器
    inputContainer_ = logicLayer_.createComponent(ComponentType::Container, rootContainer_);
    Logger::getInstance().info("创建输入区域容器，handle=" + std::to_string(inputContainer_.index));
    logicLayer_.setProperty(inputContainer_, PropertyId::Width, PropertyValue(800.0f));
    logicLayer_.setProperty(inputContainer_, PropertyId::Height, PropertyValue(60.0f));
    logicLayer_.setProperty(inputContainer_, PropertyId::FlexDirection, PropertyValue(static_cast<int>(FlexDirection::Row)));
    logicLayer_.setProperty(inputContainer_, PropertyId::PaddingLeft, PropertyValue(20.0f));
    logicLayer_.setProperty(inputContainer_, PropertyId::PaddingRight, PropertyValue(20.0f));
    logicLayer_.setProperty(inputContainer_, PropertyId::FlexShrink, PropertyValue(1.0f));
    
    // 创建输入框
    inputBox_ = logicLayer_.createComponent(ComponentType::Input, inputContainer_);
    Logger::getInstance().info("创建输入框，handle=" + std::to_string(inputBox_.index));
    logicLayer_.setProperty(inputBox_, PropertyId::Width, PropertyValue(600.0f));
    logicLayer_.setProperty(inputBox_, PropertyId::Height, PropertyValue(40.0f));
    logicLayer_.setProperty(inputBox_, PropertyId::PaddingLeft, PropertyValue(10.0f));
    logicLayer_.setProperty(inputBox_, PropertyId::PaddingTop, PropertyValue(10.0f));
    logicLayer_.setProperty(inputBox_, PropertyId::FlexShrink, PropertyValue(1.0f));
    
    // 创建添加按钮
    addButton_ = logicLayer_.createComponent(ComponentType::Button, inputContainer_);
    Logger::getInstance().info("创建添加按钮，handle=" + std::to_string(addButton_.index) + " 父容器=" + std::to_string(inputContainer_.index));
    logicLayer_.setProperty(addButton_, PropertyId::Text, PropertyValue(std::string("添加")));
    logicLayer_.setProperty(addButton_, PropertyId::Width, PropertyValue(100.0f));
    logicLayer_.setProperty(addButton_, PropertyId::Height, PropertyValue(40.0f));
    logicLayer_.setProperty(addButton_, PropertyId::MarginLeft, PropertyValue(10.0f));
    
    // 创建待办列表容器
    todoListContainer_ = logicLayer_.createComponent(ComponentType::Container, rootContainer_);
    Logger::getInstance().info("创建待办列表容器，handle=" + std::to_string(todoListContainer_.index));
    logicLayer_.setProperty(todoListContainer_, PropertyId::Width, PropertyValue(800.0f));
    logicLayer_.setProperty(todoListContainer_, PropertyId::Height, PropertyValue(400.0f));
    logicLayer_.setProperty(todoListContainer_, PropertyId::FlexDirection, PropertyValue(static_cast<int>(FlexDirection::Column)));
    logicLayer_.setProperty(todoListContainer_, PropertyId::PaddingLeft, PropertyValue(20.0f));
    logicLayer_.setProperty(todoListContainer_, PropertyId::PaddingRight, PropertyValue(20.0f));
    logicLayer_.setProperty(todoListContainer_, PropertyId::FlexShrink, PropertyValue(1.0f));
    
    // 创建统计信息
    statsText_ = logicLayer_.createComponent(ComponentType::Text, rootContainer_);
    Logger::getInstance().info("创建统计信息，handle=" + std::to_string(statsText_.index));
    logicLayer_.setProperty(statsText_, PropertyId::Text, PropertyValue(std::string("总计: 0 | 已完成: 0 | 未完成: 0")));
    logicLayer_.setProperty(statsText_, PropertyId::Width, PropertyValue(800.0f));
    logicLayer_.setProperty(statsText_, PropertyId::Height, PropertyValue(40.0f));
    logicLayer_.setProperty(statsText_, PropertyId::PaddingLeft, PropertyValue(20.0f));
    logicLayer_.setProperty(statsText_, PropertyId::PaddingTop, PropertyValue(10.0f));
    logicLayer_.setProperty(statsText_, PropertyId::FlexShrink, PropertyValue(1.0f));
    
    // 运行一帧更新
    Logger::getInstance().info("开始计算布局...");
    logicLayer_.runFrame();
    Logger::getInstance().info("TodoApp::initialize() 完成");
}

/**
 * 添加新的待办项
 * @param text 待办文本内容
 */
void TodoApp::addTodo(const std::string& text) {
    if (text.empty()) {
        return;
    }
    
    // 创建待办项数据
    TodoItem item;
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
void TodoApp::createTodoItemUI(size_t index) {
    auto& todo = todos_[index];
    
    // 创建待办项容器
    todo.handle = logicLayer_.createComponent(ComponentType::Container, todoListContainer_);
    logicLayer_.setProperty(todo.handle, PropertyId::Width, PropertyValue(760.0f));
    logicLayer_.setProperty(todo.handle, PropertyId::Height, PropertyValue(50.0f));
    logicLayer_.setProperty(todo.handle, PropertyId::FlexDirection, PropertyValue(static_cast<int>(FlexDirection::Row)));
    
    // 创建复选框（用按钮模拟）
    todo.checkbox = logicLayer_.createComponent(ComponentType::Button, todo.handle);
    logicLayer_.setProperty(todo.checkbox, PropertyId::Text, PropertyValue(std::string("[ ]")));
    logicLayer_.setProperty(todo.checkbox, PropertyId::Width, PropertyValue(40.0f));
    logicLayer_.setProperty(todo.checkbox, PropertyId::Height, PropertyValue(40.0f));
    
    // 创建文本组件
    todo.textComp = logicLayer_.createComponent(ComponentType::Text, todo.handle);
    logicLayer_.setProperty(todo.textComp, PropertyId::Text, PropertyValue(todo.text));
    logicLayer_.setProperty(todo.textComp, PropertyId::Width, PropertyValue(650.0f));
    logicLayer_.setProperty(todo.textComp, PropertyId::Height, PropertyValue(40.0f));
    logicLayer_.setProperty(todo.textComp, PropertyId::PaddingLeft, PropertyValue(10.0f));
    logicLayer_.setProperty(todo.textComp, PropertyId::PaddingTop, PropertyValue(10.0f));
    
    // 创建删除按钮
    todo.deleteBtn = logicLayer_.createComponent(ComponentType::Button, todo.handle);
    logicLayer_.setProperty(todo.deleteBtn, PropertyId::Text, PropertyValue(std::string("删除")));
    logicLayer_.setProperty(todo.deleteBtn, PropertyId::Width, PropertyValue(60.0f));
    logicLayer_.setProperty(todo.deleteBtn, PropertyId::Height, PropertyValue(40.0f));
    logicLayer_.setProperty(todo.deleteBtn, PropertyId::MarginLeft, PropertyValue(10.0f));
    
    // 运行一帧更新
    logicLayer_.runFrame();
}

/**
 * 切换待办项的完成状态
 * @param index 待办项索引
 */
void TodoApp::toggleTodo(size_t index) {
    if (index >= todos_.size()) {
        return;
    }
    
    auto& todo = todos_[index];
    todo.completed = !todo.completed;
    
    // 更新复选框显示
    if (todo.completed) {
        logicLayer_.setProperty(todo.checkbox, PropertyId::Text, PropertyValue(std::string("[x]")));
        // 完成时添加删除线样式
        std::string crossedText = std::string("[已完成] ") + todo.text;
        logicLayer_.setProperty(todo.textComp, PropertyId::Text, PropertyValue(crossedText));
    } else {
        logicLayer_.setProperty(todo.checkbox, PropertyId::Text, PropertyValue(std::string("[ ]")));
        logicLayer_.setProperty(todo.textComp, PropertyId::Text, PropertyValue(todo.text));
    }
    
    // 更新统计信息
    updateStats();
}

/**
 * 删除待办项
 * @param index 待办项索引
 */
void TodoApp::deleteTodo(size_t index) {
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
void TodoApp::updateStats() {
    std::ostringstream oss;
    oss << "总计: " << getTotalCount() 
        << " | 已完成: " << getCompletedCount()
        << " | 未完成: " << getPendingCount();
    
    logicLayer_.setProperty(statsText_, PropertyId::Text, PropertyValue(oss.str()));
}

/**
 * 获取已完成的待办数
 * @return 已完成数
 */
size_t TodoApp::getCompletedCount() const {
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
bool TodoApp::isComponentClicked(ComponentHandle handle, float x, float y) {
    if (!handle.isValid()) {
        return false;
    }
    
    auto* entry = logicLayer_.getStorage().getComponent(handle);
    if (!entry) {
        return false;
    }
    
    const auto& rect = entry->layoutResult;
    return x >= rect.x && x < rect.x + rect.width &&
           y >= rect.y && y < rect.y + rect.height;
}

/**
 * 更新每帧的事件处理
 * 检查事件日志并响应
 */
void TodoApp::update() {
    // 获取事件分发器
    auto& dispatcher = logicLayer_.getEventDispatcher();
    auto& eventLog = dispatcher.getEventLog();
    
    // 处理事件
    for (const auto& event : eventLog) {
        if (event.type == EventType::Click) {
            float x = event.mouse.position.x;
            float y = event.mouse.position.y;
            
            // 检查添加按钮
            if (isComponentClicked(addButton_, x, y)) {
                addTodo(currentInputText_);
                currentInputText_.clear();
                logicLayer_.setProperty(inputBox_, PropertyId::Text, PropertyValue(std::string()));
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
        } else if (event.type == EventType::TextInput) {
            currentInputText_ += event.text;
            logicLayer_.setProperty(inputBox_, PropertyId::Text, PropertyValue(currentInputText_));
        }
    }
    
    // 清空事件日志
    dispatcher.clearEventLog();
}

} // namespace aether
