// EventDispatcher.cpp
// 事件分发器模块 - 负责处理用户输入事件的分发
//
// 功能:
// - 鼠标事件 - 处理鼠标移动、点击、按下、释放等
// - 键盘事件 - 处理按键按下、释放等
// - 文本输入 - 处理文本输入
// - 命中检测 - 使用四叉树快速找到鼠标下的组件
// - 事件记录 - 支持记录和重放事件序列

#include "aether/EventDispatcher.h"
#include <algorithm>
#include <chrono>

namespace aether {

// 事件分发器构造函数
// 参数: storage - 组件存储引用
EventDispatcher::EventDispatcher(ComponentStorage& storage)
    : storage_(storage), quadTree_(Rect{0, 0, 1920, 1080}) {
}

// 布局完成后的回调 - 重建四叉树索引
void EventDispatcher::onLayoutComplete() {
    std::vector<ComponentHandle> interactive;
    
    // 使用forEach收集所有非容器、可见且启用的组件
    storage_.forEach([this, &interactive](ComponentHandle h) {
        auto* entry = storage_.getComponent(h);
        if (entry && entry->visible && entry->enabled) {
            if (entry->type != ComponentType::Container) {
                interactive.push_back(h);
            }
        }
    });
    
    // 重建四叉树
    quadTree_.rebuild(interactive, [this](ComponentHandle h) {
        auto* entry = storage_.getComponent(h);
        return entry ? entry->layoutResult : Rect{0, 0, 0, 0};
    });
}

// 命中检测 - 找到鼠标位置下最上层的组件
// 参数: p - 鼠标位置
// 返回值: 最上层的组件句柄
ComponentHandle EventDispatcher::hitTest(const Point& p) {
    auto candidates = quadTree_.query(p);
    
    // 反向遍历（最上层最后插入）
    for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
        auto* entry = storage_.getComponent(*it);
        if (entry && entry->visible && entry->enabled) {
            if (entry->layoutResult.contains(p)) {
                return *it;
            }
        }
    }
    
    return ComponentHandle{};
}

// 命中检测 - 找到鼠标位置下的所有组件
// 参数: p - 鼠标位置
// 返回值: 所有命中的组件列表（从上到下）
std::vector<ComponentHandle> EventDispatcher::hitTestAll(const Point& p) {
    auto candidates = quadTree_.query(p);
    std::vector<ComponentHandle> result;
    
    // 反向遍历收集
    for (auto it = candidates.rbegin(); it != candidates.rend(); ++it) {
        auto* entry = storage_.getComponent(*it);
        if (entry && entry->visible && entry->enabled) {
            if (entry->layoutResult.contains(p)) {
                result.push_back(*it);
            }
        }
    }
    
    return result;
}

// 找到事件目标
// 参数: p - 鼠标位置
// 返回值: 目标组件句柄
ComponentHandle EventDispatcher::findTarget(const Point& p) {
    return hitTest(p);
}

// 记录事件（如果正在记录）
// 参数: event - 要记录的事件
void EventDispatcher::recordEvent(const Event& event) {
    if (!isRecording_) return;
    
    RecordedEvent rec;
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
void EventDispatcher::fireEvent(ComponentHandle target, Event& event) {
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
void EventDispatcher::dispatchEvent(Event& event) {
    if (!event.target.isValid()) return;
    
    ComponentHandle current = event.target;
    
    // 冒泡遍历祖先组件
    while (current.isValid() && event.bubbles) {
        fireEvent(current, event);
        
        if (event.handled) break;
        
        auto* entry = storage_.getComponent(current);
        if (!entry || entry->parentIndex < 0) break;
        
        current = ComponentHandle{entry->parentIndex, storage_.getComponent(current) ? 
            storage_.getComponent(current)->generation : 0};
    }
}

// 分发鼠标事件
// 参数: mouseEvent - 鼠标事件对象
void EventDispatcher::dispatchMouseEvent(const MouseEvent& mouseEvent) {
    Event event;
    event.type = mouseEvent.type;
    event.mouse = mouseEvent;
    
    Point p{mouseEvent.position.x, mouseEvent.position.y};
    
    // 处理鼠标移动事件 - 检测进入和离开
    if (mouseEvent.type == EventType::MouseMove) {
        ComponentHandle newHovered = hitTest(p);
        
        if (newHovered != hoveredComponent_) {
            // 发送鼠标离开事件
            if (hoveredComponent_.isValid()) {
                Event leaveEvent;
                leaveEvent.type = EventType::MouseLeave;
                leaveEvent.mouse.position = p;
                fireEvent(hoveredComponent_, leaveEvent);
            }
            
            // 发送鼠标进入事件
            if (newHovered.isValid()) {
                Event enterEvent;
                enterEvent.type = EventType::MouseEnter;
                enterEvent.mouse.position = p;
                fireEvent(newHovered, enterEvent);
            }
            
            hoveredComponent_ = newHovered;
        }
    }
    
    // 分发事件到目标
    ComponentHandle target = findTarget(p);
    if (target.isValid()) {
        event.target = target;
        dispatchEvent(event);
        
        if (mouseCallback_) {
            mouseCallback_(event);
        }
    }
    
    // 处理点击和按下事件 - 设置焦点
    if (mouseEvent.type == EventType::Click || mouseEvent.type == EventType::MouseDown) {
        if (target.isValid()) {
            focusedComponent_ = target;
            
            Event focusEvent;
            focusEvent.type = EventType::Focus;
            focusEvent.target = target;
            fireEvent(target, focusEvent);
        }
    }
}

// 分发键盘事件
// 参数: keyEvent - 键盘事件对象
void EventDispatcher::dispatchKeyEvent(const KeyEvent& keyEvent) {
    Event event;
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
void EventDispatcher::dispatchTextInput(const std::string& text) {
    if (!focusedComponent_.isValid()) return;
    
    Event event;
    event.type = EventType::TextInput;
    event.text = text;
    event.target = focusedComponent_;
    
    dispatchEvent(event);
}

// 处理鼠标移动
// 参数: x, y - 鼠标坐标
void EventDispatcher::onMouseMove(float x, float y) {
    // 记录鼠标位置
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    MouseEvent event;
    event.type = EventType::MouseMove;
    event.position = Point{x, y};
    dispatchMouseEvent(event);
}

// 处理鼠标按下
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void EventDispatcher::onMouseDown(float x, float y, int button) {
    // 记录鼠标位置
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    MouseEvent event;
    event.type = EventType::MouseDown;
    event.position = Point{x, y};
    event.button = button;
    dispatchMouseEvent(event);
}

// 处理鼠标释放
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void EventDispatcher::onMouseUp(float x, float y, int button) {
    MouseEvent event;
    event.type = EventType::MouseUp;
    event.position = Point{x, y};
    event.button = button;
    dispatchMouseEvent(event);
}

// 处理鼠标点击
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void EventDispatcher::onClick(float x, float y, int button) {
    // 记录鼠标位置
    lastMouseX_ = x;
    lastMouseY_ = y;
    
    MouseEvent event;
    event.type = EventType::Click;
    event.position = Point{x, y};
    event.button = button;
    dispatchMouseEvent(event);
}

// 处理按键按下
// 参数: keyCode - 按键码
void EventDispatcher::onKeyDown(int keyCode) {
    KeyEvent event;
    event.type = EventType::KeyDown;
    event.keyCode = keyCode;
    dispatchKeyEvent(event);
}

// 处理按键释放
// 参数: keyCode - 按键码
void EventDispatcher::onKeyUp(int keyCode) {
    KeyEvent event;
    event.type = EventType::KeyUp;
    event.keyCode = keyCode;
    dispatchKeyEvent(event);
}

// 处理文本输入
// 参数: text - 输入的文本
void EventDispatcher::onTextInput(const std::string& text) {
    dispatchTextInput(text);
}

// 开始记录事件
// 参数: sessionId - 会话ID
void EventDispatcher::startRecording(const std::string& sessionId) {
    isRecording_ = true;
    currentSessionId_ = sessionId;
    currentRecording_.clear();
}

// 停止记录事件
// 返回值: 会话ID
std::string EventDispatcher::stopRecording() {
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
void EventDispatcher::playEvents(const std::vector<RecordedEvent>& events) {
    for (const auto& recEvent : events) {
        currentTime_ = recEvent.timestamp;
        
        // 根据事件类型分发
        if (recEvent.eventType == EventType::Click || 
            recEvent.eventType == EventType::MouseDown ||
            recEvent.eventType == EventType::MouseUp ||
            recEvent.eventType == EventType::MouseMove) {
            dispatchMouseEvent(recEvent.mouseEvent);
        } else if (recEvent.eventType == EventType::KeyDown ||
                   recEvent.eventType == EventType::KeyUp) {
            dispatchKeyEvent(recEvent.keyEvent);
        } else if (recEvent.eventType == EventType::TextInput) {
            dispatchTextInput(recEvent.textData);
        }
    }
}

} // namespace aether
