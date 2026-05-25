// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// JEventDispatcher.cpp
// 事件分发器模块 - 负责处理用户输入事件的分发
//
// 功能:
// - 鼠标事件 - 处理鼠标移动、点击、按下、释放等
// - 键盘事件 - 处理按键按下、释放等
// - 文本输入 - 处理文本输入
// - 命中检测 - 使用四叉树快速找到鼠标下的组件
// - 事件记录 - 支持记录和重放事件序列

#include "aether/EventDispatcher.h"
#include "aether/Logger.h"
#include <algorithm>
#include <chrono>

namespace jaether {

// 事件分发器构造函数
// 参数: storage - 组件存储引用
JEventDispatcher::JEventDispatcher(JComponentStorage& storage)
    : storage_(storage), quadTree_(JRect{0, 0, 1920, 1080}) {
}

// 布局完成后的回调 - 重建四叉树索引
void JEventDispatcher::onLayoutComplete() {
    std::vector<JComponentHandle> interactive;
    
    // 使用forEach收集所有非容器、可见且启用的组件
    storage_.forEach([this, &interactive](JComponentHandle h) {
        auto* entry = storage_.getComponent(h);
        if (entry && entry->visible && entry->enabled) {
            if (entry->type != JComponentType::Container) {
                interactive.push_back(h);
            }
        }
    });
    
    // 重建四叉树 - 使用绝对位置
    quadTree_.rebuild(interactive, [this](JComponentHandle h) {
        return storage_.getAbsoluteBounds(h);
    });
}

// 命中检测 - 找到鼠标位置下最上层的组件
// 参数: p - 鼠标位置
// 返回值: 最上层的组件句柄；如果命中的是控件子元素（如Button内的Text），
//         自动上溯到交互控件本身
JComponentHandle JEventDispatcher::hitTest(const JPoint& p) {
    JLogger::getInstance().info("[hitTest] 开始命中测试，鼠标位置: (" + 
        std::to_string(static_cast<int>(p.x)) + ", " + 
        std::to_string(static_cast<int>(p.y)) + ")");
    
    auto candidates = quadTree_.query(p);
    JLogger::getInstance().info("[hitTest] 四叉树查询到 " + 
        std::to_string(candidates.size()) + " 个候选组件");
    
    // 反向遍历（最上层最后插入）
    for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
        auto* entry = storage_.getComponent(*it);
        if (entry && entry->visible && entry->enabled) {
            // 使用绝对位置进行检查
            JRect absoluteBounds = storage_.getAbsoluteBounds(*it);
            if (absoluteBounds.contains(p)) {
                // 命中组件后，向上查找交互控件祖先
                // 如果命中Text/Card内部Text等叶子节点，
                // 自动上溯到最近的Button/Input/Card等交互控件
                JComponentHandle result = findInteractiveAncestor(*it);
                JLogger::getInstance().info("[hitTest] 找到命中组件: index=" + 
                    std::to_string(result.index));
                return result;
            }
        }
    }
    
    JLogger::getInstance().info("[hitTest] 未找到命中组件");
    return JComponentHandle{};
}

// 查找最近的交互控件祖先
// 当命中Text等非交互叶子时，上溯到最近的Button/Input/Card等控件
JComponentHandle JEventDispatcher::findInteractiveAncestor(JComponentHandle h) {
    auto* entry = storage_.getComponent(h);
    if (!entry) return h;
    
    // 如果当前组件不是Text，不需要上溯
    if (entry->type != JComponentType::Text) return h;
    
    // 沿父链向上查找，直到找到Button/Input/Card或到达根组件
    int32_t idx = h.index;
    for (int depth = 0; depth < 10; ++depth) {
        auto* cur = storage_.getComponentByIndex(idx);
        if (!cur || cur->parentIndex < 0) break;
        
        auto* parent = storage_.getComponentByIndex(cur->parentIndex);
        if (!parent) break;
        
        // 检查父组件是否是交互控件
        if (parent->type == JComponentType::Button ||
            parent->type == JComponentType::Input ||
            parent->type == JComponentType::Card) {
            return JComponentHandle{cur->parentIndex, parent->generation};
        }
        
        // 继续向上（通过Container链）
        idx = cur->parentIndex;
    }
    
    return h;  // 未找到交互祖先，返回原始句柄
}

// 命中检测 - 找到鼠标位置下的所有组件
// 参数: p - 鼠标位置
// 返回值: 所有命中的组件列表（从上到下）
std::vector<JComponentHandle> JEventDispatcher::hitTestAll(const JPoint& p) {
    auto candidates = quadTree_.query(p);
    std::vector<JComponentHandle> result;
    
    // 反向遍历收集
    for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
        auto* entry = storage_.getComponent(*it);
        if (entry && entry->visible && entry->enabled) {
            // 使用绝对位置进行检查
            JRect absoluteBounds = storage_.getAbsoluteBounds(*it);
            if (absoluteBounds.contains(p)) {
                result.push_back(*it);
            }
        }
    }
    
    return result;
}

// 找到事件目标
// 参数: p - 鼠标位置
// 返回值: 目标组件句柄
JComponentHandle JEventDispatcher::findTarget(const JPoint& p) {
    return hitTest(p);
}

// 记录事件（如果正在记录）
// 参数: event - 要记录的事件
void JEventDispatcher::recordEvent(const JEvent& event) {
    if (!isRecording_) return;
    
    JRecordedEvent rec;
    rec.timestamp = currentTime_;
    rec.eventType = event.type;
    rec.mouseEvent = event.mouse;
    rec.keyEvent = event.key;
    rec.textData = event.text;
    
    currentRecording_.push_back(rec);
}

// 向目标组件发送事件
// 参数:
//   target - 目标组件
//   event - 事件对象
void JEventDispatcher::fireEvent(JComponentHandle target, JEvent& event) {
    event.target = target;
    event.currentTarget = target;
    
    // 调用鼠标回调
    auto* entry = storage_.getComponent(target);
    if (entry && mouseCallback_) {
        mouseCallback_(event);
    }
    
    // 记录到日志
    eventLog_.push_back(event);
    
    // 记录事件（如果正在记录）
    recordEvent(event);
}

// 分发事件，支持冒泡
// 参数: event - 事件对象
void JEventDispatcher::dispatchEvent(JEvent& event) {
    if (!event.target.isValid()) return;
    
    JComponentHandle current = event.target;
    
    // 冒泡遍历祖先组件
    while (current.isValid() && event.bubbles) {
        fireEvent(current, event);
        
        if (event.handled) break;
        
        auto* entry = storage_.getComponent(current);
        if (!entry || entry->parentIndex < 0) break;
        
        current = JComponentHandle{entry->parentIndex, storage_.getComponent(current) ? 
            storage_.getComponent(current)->generation : 0};
    }
}

// 分发鼠标事件
// 参数: mouseEvent - 鼠标事件对象
void JEventDispatcher::dispatchMouseEvent(const JMouseEvent& mouseEvent) {
    JEvent event;
    event.type = mouseEvent.type;
    event.mouse = mouseEvent;
    
    JPoint p{mouseEvent.position.x, mouseEvent.position.y};
    
    // 处理鼠标移动事件 - 检测进入和离开
    if (mouseEvent.type == JEventType::MouseMove) {
        JComponentHandle newHovered = hitTest(p);
        
        if (newHovered != hoveredComponent_) {
            // 发送鼠标离开事件
            if (hoveredComponent_.isValid()) {
                JEvent leaveEvent;
                leaveEvent.type = JEventType::MouseLeave;
                leaveEvent.mouse.position = p;
                fireEvent(hoveredComponent_, leaveEvent);
            }
            
            // 发送鼠标进入事件
            if (newHovered.isValid()) {
                JEvent enterEvent;
                enterEvent.type = JEventType::MouseEnter;
                enterEvent.mouse.position = p;
                fireEvent(newHovered, enterEvent);
            }
            
            hoveredComponent_ = newHovered;
        }
    }
    
    // 分发事件到目标
    JComponentHandle target = findTarget(p);
    if (target.isValid()) {
        event.target = target;
        dispatchEvent(event);
        
        if (mouseCallback_) {
            mouseCallback_(event);
        }
    } else {
        // 即使没有目标，也要记录事件到日志
        eventLog_.push_back(event);
    }
    
    // 处理点击和按下事件 - 设置焦点
    if (mouseEvent.type == JEventType::Click || mouseEvent.type == JEventType::MouseDown) {
        if (target.isValid()) {
            focusedComponent_ = target;
            
            JEvent focusEvent;
            focusEvent.type = JEventType::Focus;
            focusEvent.target = target;
            fireEvent(target, focusEvent);
        }
    }
}

// 分发键盘事件
// 参数: keyEvent - 键盘事件对象
void JEventDispatcher::dispatchKeyEvent(const JKeyEvent& keyEvent) {
    JEvent event;
    event.type = keyEvent.type;
    event.key = keyEvent;
    
    // 分发到焦点组件
    if (focusedComponent_.isValid()) {
        event.target = focusedComponent_;
        dispatchEvent(event);
        
        if (keyCallback_) {
            keyCallback_(event);
        }
    }
}

// 分发文本输入事件
// 参数: text - 输入的文本
void JEventDispatcher::dispatchTextInput(const std::string& text) {
    JEvent event;
    event.type = JEventType::TextInput;
    event.text = text;
    
    if (focusedComponent_.isValid()) {
        event.target = focusedComponent_;
        dispatchEvent(event);
    } else {
        // 即使没有焦点组件，也要记录事件到日志
        eventLog_.push_back(event);
    }
}

// 处理鼠标移动
// 参数: x, y - 鼠标坐标
void JEventDispatcher::onMouseMove(float x, float y) {
    // 记录鼠标位置
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    JMouseEvent event;
    event.type = JEventType::MouseMove;
    event.position = JPoint{x, y};
    dispatchMouseEvent(event);
}

// 处理鼠标按下
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JEventDispatcher::onMouseDown(float x, float y, int button) {
    // 记录鼠标位置
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    JMouseEvent event;
    event.type = JEventType::MouseDown;
    event.position = JPoint{x, y};
    event.button = button;
    dispatchMouseEvent(event);
}

// 处理鼠标释放
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JEventDispatcher::onMouseUp(float x, float y, int button) {
    JMouseEvent event;
    event.type = JEventType::MouseUp;
    event.position = JPoint{x, y};
    event.button = button;
    dispatchMouseEvent(event);
}

// 处理鼠标点击
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JEventDispatcher::onClick(float x, float y, int button) {
    // 记录鼠标位置
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    JMouseEvent event;
    event.type = JEventType::Click;
    event.position = JPoint{x, y};
    event.button = button;
    dispatchMouseEvent(event);
}

// 处理按键按下
// 参数: keyCode - 按键码
void JEventDispatcher::onKeyDown(int keyCode) {
    JKeyEvent event;
    event.type = JEventType::KeyDown;
    event.keyCode = keyCode;
    dispatchKeyEvent(event);
}

// 处理按键释放
// 参数: keyCode - 按键码
void JEventDispatcher::onKeyUp(int keyCode) {
    JKeyEvent event;
    event.type = JEventType::KeyUp;
    event.keyCode = keyCode;
    dispatchKeyEvent(event);
}

// 处理文本输入
// 参数: text - 输入的文本
void JEventDispatcher::onTextInput(const std::string& text) {
    dispatchTextInput(text);
}

// 开始记录事件
// 参数: sessionId - 会话ID
void JEventDispatcher::startRecording(const std::string& sessionId) {
    isRecording_ = true;
    currentSessionId_ = sessionId;
    currentRecording_.clear();
}

// 停止记录事件
// 返回值: 会话ID
std::string JEventDispatcher::stopRecording() {
    if (!isRecording_) {
        return "";
    }
    
    isRecording_ = false;
    recordedSessions_[currentSessionId_] = currentRecording_;
    std::string sessionId = currentSessionId_;
    currentSessionId_ = "";
    
    return sessionId;
}

// 播放录制的事件
// 参数: events - 要播放的事件列表
void JEventDispatcher::playEvents(const std::vector<JRecordedEvent>& events) {
    for (const auto& recEvent : events) {
        currentTime_ = recEvent.timestamp;
        
        // 根据事件类型分发
        if (recEvent.eventType == JEventType::Click || 
            recEvent.eventType == JEventType::MouseDown ||
            recEvent.eventType == JEventType::MouseUp ||
            recEvent.eventType == JEventType::MouseMove) {
            dispatchMouseEvent(recEvent.mouseEvent);
        } else if (recEvent.eventType == JEventType::KeyDown ||
                   recEvent.eventType == JEventType::KeyUp) {
            dispatchKeyEvent(recEvent.keyEvent);
        } else if (recEvent.eventType == JEventType::TextInput) {
            dispatchTextInput(recEvent.textData);
        }
    }
}

} // namespace jaether
