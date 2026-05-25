// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include "QuadTree.h"
#include <functional>
#include <vector>
#include <string>
#include <map>

namespace jaether {

/**
 * 事件类型枚举
 * 
 * 定义了所有支持的UI事件类型
 */
enum class JEventType {
    Click,          // 点击事件
    MouseDown,      // 鼠标按下
    MouseUp,        // 鼠标释放
    MouseMove,      // 鼠标移动
    MouseEnter,     // 鼠标进入
    MouseLeave,     // 鼠标离开
    KeyDown,        // 键盘按下
    KeyUp,          // 键盘释放
    TextInput,      // 文本输入
    Focus,          // 获得焦点
    Blur,           // 失去焦点
    Change          // 值改变
};

/**
 * 鼠标事件结构
 * 
 * 包含鼠标事件相关的信息
 */
struct JMouseEvent {
    JEventType type = JEventType::Click;  // 事件类型
    JPoint position;                     // 鼠标位置
    int button = 0;                     // 鼠标按钮（0-左，1-中，2-右）
    int clickCount = 1;                  // 点击次数
};

/**
 * 键盘事件结构
 * 
 * 包含键盘事件相关的信息
 */
struct JKeyEvent {
    JEventType type = JEventType::KeyDown;  // 事件类型
    int keyCode = 0;                      // 键码
    int modifiers = 0;                   // 修饰键（Shift/Ctrl/Alt等）
    char text = 0;                        // 输入字符
};

/**
 * 通用事件结构
 * 
 * 包含事件的所有信息
 */
struct JEvent {
    JEventType type;                   // 事件类型
    JComponentHandle target;           // 事件目标组件
    JComponentHandle currentTarget;    // 当前处理事件的组件
    JMouseEvent mouse;                 // 鼠标事件信息（如果是鼠标事件）
    JKeyEvent key;                     // 键盘事件信息（如果是键盘事件）
    std::string text;                 // 文本数据（如果是文本事件）
    bool handled = false;             // 事件是否已处理
    bool bubbles = true;              // 是否冒泡
};

/**
 * 事件回调类型
 */
using JEventCallback = std::function<void(JEvent&)>;

/**
 * 已记录的事件结构
 * 
 * 用于事件回放和测试
 */
struct JRecordedEvent {
    int64_t timestamp;                // 时间戳
    JEventType eventType;              // 事件类型
    JMouseEvent mouseEvent;            // 鼠标事件数据
    JKeyEvent keyEvent;                // 键盘事件数据
    std::string textData;             // 文本数据
};

/**
 * 事件分发器类
 * 
 * 负责处理UI事件的分发
 * 支持事件冒泡、事件监听、事件记录和回放
 * 使用四叉树进行高效的命中测试
 */
class JEventDispatcher {
public:
    /**
     * 构造函数
     * @param storage 组件存储引用
     */
    explicit JEventDispatcher(JComponentStorage& storage);
    
    /**
     * 布局完成回调
     * 在布局引擎完成布局后调用
     */
    void onLayoutComplete();
    
    /**
     * 命中测试 - 查找指定位置的顶层组件
     * @param p 测试点
     * @return 命中的组件句柄
     */
    JComponentHandle hitTest(const JPoint& p);
    
    /**
     * 命中测试 - 查找指定位置的所有组件
     * @param p 测试点
     * @return 所有命中的组件列表（从下到上）
     */
    std::vector<JComponentHandle> hitTestAll(const JPoint& p);
    
    /**
     * 分发鼠标事件
     * @param event 鼠标事件
     */
    void dispatchMouseEvent(const JMouseEvent& event);
    
    /**
     * 分发键盘事件
     * @param event 键盘事件
     */
    void dispatchKeyEvent(const JKeyEvent& event);
    
    /**
     * 分发文本输入事件
     * @param text 输入的文本
     */
    void dispatchTextInput(const std::string& text);
    
    /**
     * 处理鼠标移动
     * @param x X坐标
     * @param y Y坐标
     */
    void onMouseMove(float x, float y);
    
    /**
     * 处理鼠标按下
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void onMouseDown(float x, float y, int button);
    
    /**
     * 处理鼠标释放
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void onMouseUp(float x, float y, int button);
    
    /**
     * 处理点击
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void onClick(float x, float y, int button);
    
    /**
     * 处理键盘按下
     * @param keyCode 键码
     */
    void onKeyDown(int keyCode);
    
    /**
     * 处理键盘释放
     * @param keyCode 键码
     */
    void onKeyUp(int keyCode);
    
    /**
     * 处理文本输入
     * @param text 输入文本
     */
    void onTextInput(const std::string& text);
    
    /**
     * 设置鼠标事件回调
     * @param callback 回调函数
     */
    void setMouseCallback(JEventCallback callback) { mouseCallback_ = std::move(callback); }
    
    /**
     * 设置键盘事件回调
     * @param callback 回调函数
     */
    void setKeyCallback(JEventCallback callback) { keyCallback_ = std::move(callback); }
    
    /**
     * 获取事件日志
     * @return 事件日志列表
     */
    const std::vector<JEvent>& getEventLog() const { return eventLog_; }
    
    /**
     * 清空事件日志
     */
    void clearEventLog() { eventLog_.clear(); }
    
    /**
     * 开始录制事件
     * @param sessionId 会话ID
     */
    void startRecording(const std::string& sessionId);
    
    /**
     * 停止录制事件
     * @return 录制的会话ID
     */
    std::string stopRecording();
    
    /**
     * 检查是否正在录制
     * @return 如果正在录制返回true
     */
    bool isRecording() const { return isRecording_; }
    
    /**
     * 获取当前会话ID
     * @return 当前会话ID
     */
    std::string getCurrentSessionId() const { return currentSessionId_; }
    
    /**
     * 回放事件
     * @param events 要回放的事件列表
     */
    void playEvents(const std::vector<JRecordedEvent>& events);
    
    /**
     * 获取已录制的所有会话
     * @return 会话映射表
     */
    const std::map<std::string, std::vector<JRecordedEvent>>& getRecordedSessions() const {
        return recordedSessions_;
    }
    
    /**
     * 设置当前时间（用于测试）
     * @param time 时间戳
     */
    void setCurrentTime(int64_t time) { currentTime_ = time; }
    
    /**
     * 获取当前时间
     * @return 当前时间戳
     */
    int64_t getCurrentTime() const { return currentTime_; }
    
    /**
     * 获取最后记录的鼠标X坐标
     * @return 鼠标X坐标
     */
    float getLastMouseX() const { return lastMouseX_; }
    
    /**
     * 获取最后记录的鼠标Y坐标
     * @return 鼠标Y坐标
     */
    float getLastMouseY() const { return lastMouseY_; }
    
private:
    /**
     * 查找事件目标组件
     * @param p 触发点
     * @return 目标组件句柄
     */
    JComponentHandle findTarget(const JPoint& p);
    
    /**
     * 分发事件
     * @param event 要分发的事件
     */
    void dispatchEvent(JEvent& event);
    
    /**
     * 向指定组件发送事件
     * @param target 目标组件
     * @param event 事件
     */
    void fireEvent(JComponentHandle target, JEvent& event);
    
    /**
     * 记录事件
     * @param event 要记录的事件
     */
    void recordEvent(const JEvent& event);
    
    JComponentStorage& storage_;              // 组件存储引用
    JQuadTree quadTree_;                      // 四叉树用于命中测试
    JComponentHandle focusedComponent_;       // 当前聚焦组件
    JComponentHandle hoveredComponent_;       // 当前悬停组件
    std::vector<JEvent> eventLog_;           // 事件日志
    JEventCallback mouseCallback_;            // 鼠标事件回调
    JEventCallback keyCallback_;              // 键盘事件回调
    
    bool isRecording_ = false;               // 是否正在录制
    std::string currentSessionId_;           // 当前会话ID
    std::map<std::string, std::vector<JRecordedEvent>> recordedSessions_;  // 已录制会话
    std::vector<JRecordedEvent> currentRecording_;  // 当前录制的事件
    int64_t currentTime_ = 0;                // 当前时间
    float lastMouseX_ = 0.0f;                // 最后记录的鼠标X坐标
    float lastMouseY_ = 0.0f;                // 最后记录的鼠标Y坐标
};

}
