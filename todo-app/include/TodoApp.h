#pragma once

#include "aether/LogicLayer.h"
#include <string>
#include <vector>
#include <map>

namespace jaether {

/**
 * 待办项数据结构
 * 
 * 存储单个待办项的信息
 */
struct JTodoItem {
    std::string text;                // 待办文本
    bool completed = false;          // 是否完成
    JComponentHandle handle = {};     // 组件句柄
    JComponentHandle checkbox = {};   // 复选框组件
    JComponentHandle textComp = {};   // 文本组件
    JComponentHandle deleteBtn = {};  // 删除按钮
};

/**
 * 待办应用类
 * 
 * 实现待办列表应用的核心逻辑
 * 管理待办项的添加、删除、状态切换
 */
class JTodoApp {
public:
    /**
     * 构造函数
     * @param logicLayer 逻辑层引用
     */
    explicit JTodoApp(JLogicLayer& logicLayer);
    
    /**
     * 析构函数
     */
    ~JTodoApp();
    
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
    JComponentHandle getRootContainer() const { return rootContainer_; }
    
    /**
     * 获取添加按钮组件句柄
     * @return 添加按钮句柄
     */
    JComponentHandle getAddButton() const { return addButton_; }
    
    /**
     * 获取输入框组件句柄
     * @return 输入框句柄
     */
    JComponentHandle getInputBox() const { return inputBox_; }
    
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
    bool isComponentClicked(JComponentHandle handle, float x, float y);
    
    JLogicLayer& logicLayer_;              // 逻辑层引用
    
    // UI组件
    JComponentHandle rootContainer_;        // 根容器
    JComponentHandle title_;                // 标题
    JComponentHandle inputContainer_;       // 输入区域容器
    JComponentHandle inputBox_;             // 输入框
    JComponentHandle addButton_;            // 添加按钮
    JComponentHandle todoListContainer_;    // 待办列表容器
    JComponentHandle statsText_;            // 统计信息
    
    std::vector<JTodoItem> todos_;          // 待办项列表
    std::string currentInputText_;         // 当前输入文本
};

} // namespace jaether
