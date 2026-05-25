// JTestController.cpp
// 测试控制器模块 - 提供UI自动化测试的控制接口
//
// 功能:
// - 组件树查询和操作
// - 属性获取和设置
// - 事件注入
// - 快照捕获
// - 条件等待
// - 事件记录和回放

#include "aether/TestController.h"
#include <chrono>
#include <sstream>
#include <thread>
#include <random>

namespace jaether {

// 测试控制器构造函数
// 参数:
//   stateManager - 状态管理器引用
//   dispatcher - 事件分发器引用
JTestController::JTestController(JStateManager& stateManager, JEventDispatcher& dispatcher)
    : stateManager_(stateManager), dispatcher_(dispatcher) {
    startTime_ = std::chrono::steady_clock::now();
}

// 生成会话ID
// 返回值: 唯一的会话ID字符串
std::string JTestController::generateSessionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    std::ostringstream oss;
    oss << "session_" << dis(gen) << "_" << std::chrono::system_clock::now().time_since_epoch().count();
    return oss.str();
}

// 获取组件树的JSON表示
// 返回值: 组件树的JSON字符串
std::string JTestController::getComponentTreeJSON() {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"components\": [\n";
    
    bool first = true;
    stateManager_.getStorage().forEach([&](JComponentHandle h) {
        auto* entry = stateManager_.getStorage().getComponent(h);
        if (!entry) return;
        
        if (!first) oss << ",\n";
        first = false;
        
        oss << "    {\n";
        oss << "      \"id\": " << entry->id << ",\n";
        oss << "      \"type\": " << static_cast<int>(entry->type) << ",\n";
        oss << "      \"visible\": " << (entry->visible ? "true" : "false") << ",\n";
        oss << "      \"enabled\": " << (entry->enabled ? "true" : "false") << ",\n";
        oss << "      \"layout\": {";
        oss << "\"x\":" << entry->layoutResult.left() << ",";
        oss << "\"y\":" << entry->layoutResult.top() << ",";
        oss << "\"width\":" << entry->layoutResult.width << ",";
        oss << "\"height\":" << entry->layoutResult.height << "},\n";
        
        // 添加文本属性（如果存在）
        auto* textProp = entry->properties.getProperty(JPropertyId::Text);
        if (textProp) {
            oss << "      \"text\": \"" << textProp->toString() << "\"\n";
        } else {
            oss << "      \"text\": \"\"\n";
        }
        
        oss << "    }";
    });
    
    oss << "\n  ]\n";
    oss << "}\n";
    
    return oss.str();
}

// 获取组件属性
// 参数:
//   idStr - 组件ID字符串
//   propStr - 属性名称字符串
// 返回值: 属性值的字符串表示
std::string JTestController::getProperty(const std::string& idStr, const std::string& propStr) {
    auto handle = findComponentByIdString(idStr);
    if (!handle.isValid()) return "null";
    
    JPropertyId id = getPropertyIdFromName(propStr);
    if (id == JPropertyId::Unknown) return "null";
    
    auto* value = stateManager_.getProperty(handle, id);
    if (!value) return "null";
    
    return value->toString();
}

// 设置组件属性
// 参数:
//   idStr - 组件ID字符串
//   propStr - 属性名称字符串
//   valueJson - 属性值JSON字符串
// 返回值: 成功返回true，否则返回false
bool JTestController::setProperty(const std::string& idStr, const std::string& propStr, const std::string& valueJson) {
    auto handle = findComponentByIdString(idStr);
    if (!handle.isValid()) return false;
    
    JPropertyId id = getPropertyIdFromName(propStr);
    if (id == JPropertyId::Unknown) return false;
    
    auto value = JPropertyValue::fromString(id, valueJson);
    stateManager_.setProperty(handle, id, value);
    
    return true;
}

// 注入鼠标事件
// 参数:
//   type - 事件类型字符串
//   x, y - 坐标
//   button - 鼠标按钮
void JTestController::injectMouseEvent(const std::string& type, int x, int y, int button) {
    JMouseEvent event;
    event.position.x = static_cast<float>(x);
    event.position.y = static_cast<float>(y);
    event.button = button;
    
    // 根据类型字符串设置事件类型
    if (type == "click") event.type = JEventType::Click;
    else if (type == "mousedown") event.type = JEventType::MouseDown;
    else if (type == "mouseup") event.type = JEventType::MouseUp;
    else if (type == "mousemove") event.type = JEventType::MouseMove;
    else return;
    
    dispatcher_.dispatchMouseEvent(event);
}

// 注入键盘事件
// 参数:
//   type - 事件类型字符串
//   keyCode - 按键码
//   modifiers - 修饰键
void JTestController::injectKeyEvent(const std::string& type, int keyCode, int modifiers) {
    JKeyEvent event;
    event.keyCode = keyCode;
    event.modifiers = modifiers;
    
    if (type == "keydown") event.type = JEventType::KeyDown;
    else if (type == "keyup") event.type = JEventType::KeyUp;
    else return;
    
    dispatcher_.dispatchKeyEvent(event);
}

// 注入文本输入
// 参数: text - 输入的文本
void JTestController::injectTextInput(const std::string& text) {
    dispatcher_.dispatchTextInput(text);
}

// 推进时间
// 参数: ms - 毫秒数
void JTestController::advanceTime(int ms) {
    currentTime_ += ms;
    dispatcher_.setCurrentTime(currentTime_);
}

// 捕获快照
// 返回值: 快照的JSON字符串
std::string JTestController::takeSnapshot() {
    auto snapshot = JSnapshotSerializer::capture(stateManager_.getStorage());
    return JSnapshotSerializer::toJSON(snapshot);
}

// 评估JSON路径
// 参数: jsonPath - JSON路径字符串
// 返回值: 路径指向的值的字符串表示
std::string JTestController::evaluateJSONPath(const std::string& jsonPath) {
    size_t pos = jsonPath.find('.');
    if (pos == std::string::npos) {
        // 只有组件ID，尝试获取value属性
        return getProperty(jsonPath, "value");
    }
    
    std::string componentId = jsonPath.substr(0, pos);
    std::string property = jsonPath.substr(pos + 1);
    
    // 处理带前缀的属性名，如 property.text 或 layout.x
    if (property.find("property") != std::string::npos || property.find("layout") != std::string::npos) {
        size_t dotPos = property.find('.');
        if (dotPos != std::string::npos) {
            std::string propName = property.substr(dotPos + 1);
            return getProperty(componentId, propName);
        }
    }
    
    return getProperty(componentId, property);
}

// 等待条件满足
// 参数:
//   jsonPath - 要检查的JSON路径
//   expectedValue - 期望值
//   timeoutMs - 超时时间（毫秒）
// 返回值: 条件满足返回true，超时返回false
bool JTestController::waitForCondition(const std::string& jsonPath, const std::string& expectedValue, int timeoutMs) {
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        // 检查是否超时
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        
        if (elapsed >= timeoutMs) return false;
        
        // 检查条件是否满足
        auto current = evaluateJSONPath(jsonPath);
        if (current == expectedValue) return true;
        
        // 等待一段时间后重试
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 获取事件日志
// 返回值: 事件日志的字符串列表
std::vector<std::string> JTestController::getEventLog() {
    std::vector<std::string> result;
    
    for (const auto& event : dispatcher_.getEventLog()) {
        std::ostringstream oss;
        oss << "JEvent: " << static_cast<int>(event.type);
        oss << " target: " << event.target.index;
        result.push_back(oss.str());
    }
    
    return result;
}

// 通过ID字符串查找组件
// 参数: idStr - 组件ID字符串（可带0x前缀表示十六进制）
// 返回值: 组件句柄
JComponentHandle JTestController::findComponentByIdString(const std::string& idStr) {
    try {
        if (idStr.find("0x") == 0) {
            // 十六进制格式
            unsigned long id = std::stoul(idStr.substr(2), nullptr, 16);
            return stateManager_.getStorage().findById(static_cast<JComponentId>(id));
        } else {
            // 十进制格式
            unsigned long id = std::stoul(idStr);
            return stateManager_.getStorage().findById(static_cast<JComponentId>(id));
        }
    } catch (...) {
        return JComponentHandle{};
    }
}

// 通过ID获取组件（同findComponentByIdString）
JComponentHandle JTestController::getComponentById(const std::string& id) {
    return findComponentByIdString(id);
}

// 开始记录事件
// 返回值: 会话ID
std::string JTestController::startRecording() {
    std::string sessionId = generateSessionId();
    dispatcher_.startRecording(sessionId);
    return sessionId;
}

// 停止记录事件
// 返回值: 会话ID
std::string JTestController::stopRecording() {
    std::string sessionId = dispatcher_.stopRecording();
    return sessionId;
}

// 回放事件
// 参数: sessionData - 会话数据（CSV格式的事件记录）
void JTestController::playback(const std::string& sessionData) {
    std::vector<JRecordedEvent> events;
    
    std::istringstream iss(sessionData);
    std::string line;
    
    // 逐行解析事件
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        
        JRecordedEvent rec;
        std::istringstream lineStream(line);
        std::string token;
        
        // 解析时间戳
        if (std::getline(lineStream, token, ',')) {
            rec.timestamp = std::stoll(token);
        }
        // 解析事件类型
        if (std::getline(lineStream, token, ',')) {
            rec.eventType = static_cast<JEventType>(std::stoi(token));
        }
        // 解析鼠标X坐标
        if (std::getline(lineStream, token, ',')) {
            rec.mouseEvent.position.x = std::stof(token);
        }
        // 解析鼠标Y坐标
        if (std::getline(lineStream, token, ',')) {
            rec.mouseEvent.position.y = std::stof(token);
        }
        // 解析鼠标按钮
        if (std::getline(lineStream, token, ',')) {
            rec.mouseEvent.button = std::stoi(token);
        }
        // 解析按键码
        if (std::getline(lineStream, token, ',')) {
            rec.keyEvent.keyCode = std::stoi(token);
        }
        // 解析文本数据
        if (std::getline(lineStream, token)) {
            rec.textData = token;
        }
        
        events.push_back(rec);
    }
    
    // 播放事件
    dispatcher_.playEvents(events);
}

} // namespace jaether
