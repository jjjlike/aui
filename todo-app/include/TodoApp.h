#pragma once

#include "aether/LogicLayer.h"
#include <string>
#include <vector>
#include <map>

namespace aether {

/**
 * 待办项数据结构
 * 
 * 存储单个待办项的信息
 */
struct TodoItem {
    std::string text;                // 待办文本
    bool completed = false;          // 是否完成
    ComponentHandle handle = {};     // 组件句柄
    ComponentHandle checkbox = {};   // 复选框组件
    ComponentHandle textComp = {};   // 文本组件
    ComponentHandle deleteBtn = {};  // 删除按钮
};

/**
 * 待办应用类
 * 
 * 实现待办列表应用的核心逻辑
 * 管理待办项的添加、删除、状态切换
 */
class TodoApp {
public:
    /**
     * 构造函数
     * @param logicLayer 逻辑层引用
     */
    explicit TodoApp(LogicLayer& logicLayer);
    
    /**
     * 析构函数
     */
    ~TodoApp();
    
    /**
     * 初始化应用UI
     * 创建所有UI组件并设置布局
     */
    void initialize();
    
    /**
     * 添加新的待办项
     * @param text 待办文本内容
     */
    void addTodo(const std::string& text);
    
    /**
     * 切换待办项的完成状态
     * @param index 待办项索引
     */
    void toggleTodo(size_t index);
    
    /**
     * 删除待办项
     * @param index 待办项索引
     */
    void deleteTodo(size_t index);
    
    /**
     * 更新每帧的事件处理
     * 检查事件日志并响应
     */
    void update();
    
    /**
     * 获取待办总数
     * @return 待办总数
     */
    size_t getTotalCount() const { return todos_.size(); }
    
    /**
     * 获取已完成的待办数
     * @return 已完成数
     */
    size_t getCompletedCount() const;
    
    /**
     * 获取未完成的待办数
     * @return 未完成数
     */
    size_t getPendingCount() const { return getTotalCount() - getCompletedCount(); }
    
    /**
     * 获取根容器组件句柄
     * @return 根容器句柄
     */
    ComponentHandle getRootContainer() const { return rootContainer_; }
    
    /**
     * 获取添加按钮组件句柄
     * @return 添加按钮句柄
     */
    ComponentHandle getAddButton() const { return addButton_; }
    
    /**
     * 获取输入框组件句柄
     * @return 输入框句柄
     */
    ComponentHandle getInputBox() const { return inputBox_; }
    
private:
    /**
     * 创建UI组件
     */
    void createUI();
    
    /**
     * 创建单个待办项的UI
     * @param index 待办项索引
     */
    void createTodoItemUI(size_t index);
    
    /**
     * 更新统计信息显示
     */
    void updateStats();
    
    /**
     * 处理用户点击事件
     */
    void handleClicks();
    
    /**
     * 处理文本输入事件
     */
    void handleTextInput();
    
    /**
     * 检查组件是否在某个位置被点击
     * @param handle 组件句柄
     * @param x 点击x坐标
     * @param y 点击y坐标
     * @return 是否点击了该组件
     */
    bool isComponentClicked(ComponentHandle handle, float x, float y);
    
    LogicLayer& logicLayer_;              // 逻辑层引用
    
    // UI组件
    ComponentHandle rootContainer_;        // 根容器
    ComponentHandle title_;                // 标题
    ComponentHandle inputContainer_;       // 输入区域容器
    ComponentHandle inputBox_;             // 输入框
    ComponentHandle addButton_;            // 添加按钮
    ComponentHandle todoListContainer_;    // 待办列表容器
    ComponentHandle statsText_;            // 统计信息
    
    std::vector<TodoItem> todos_;          // 待办项列表
    std::string currentInputText_;         // 当前输入文本
};

} // namespace aether
