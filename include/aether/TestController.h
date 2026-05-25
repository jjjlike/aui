#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include "StateManager.h"
#include "EventDispatcher.h"
#include "Snapshot.h"
#include <string>
#include <vector>
#include <chrono>
#include <memory>

namespace jaether {

// 前向声明
class JA2UIGenerator;
class JJSurfaceManager;

/**
 * 测试控制器接口
 * 
 * 定义了自动化测试所需的核心功能
 */
class ITestController {
public:
    virtual ~ITestController() = default;
    
    /**
     * 获取组件树JSON
     * @return JSON字符串
     */
    virtual std::string getComponentTreeJSON() = 0;
    
    /**
     * 获取组件属性
     * @param id 组件ID（字符串形式）
     * @param prop 属性名
     * @return 属性值的JSON字符串
     */
    virtual std::string getProperty(const std::string& id, const std::string& prop) = 0;
    
    /**
     * 设置组件属性
     * @param id 组件ID（字符串形式）
     * @param prop 属性名
     * @param valueJson 属性值的JSON字符串
     * @return 是否成功
     */
    virtual bool setProperty(const std::string& id, const std::string& prop, const std::string& valueJson) = 0;
    
    /**
     * 注入鼠标事件
     * @param type 事件类型
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    virtual void injectMouseEvent(const std::string& type, int x, int y, int button) = 0;
    
    /**
     * 注入键盘事件
     * @param type 事件类型
     * @param keyCode 键码
     * @param modifiers 修饰键
     */
    virtual void injectKeyEvent(const std::string& type, int keyCode, int modifiers) = 0;
    
    /**
     * 注入文本输入
     * @param text 输入文本
     */
    virtual void injectTextInput(const std::string& text) = 0;
    
    /**
     * 推进时间（用于动画和延迟测试）
     * @param ms 毫秒数
     */
    virtual void advanceTime(int ms) = 0;
    
    /**
     * 拍摄快照
     * @return 快照JSON字符串
     */
    virtual std::string takeSnapshot() = 0;
    
    /**
     * 等待条件满足
     * @param jsonPath JSON路径表达式
     * @param expectedValue 期望值
     * @param timeoutMs 超时时间（毫秒）
     * @return 条件是否满足
     */
    virtual bool waitForCondition(const std::string& jsonPath, const std::string& expectedValue, int timeoutMs) = 0;
    
    /**
     * 获取事件日志
     * @return 事件日志列表
     */
    virtual std::vector<std::string> getEventLog() = 0;
    
    /**
     * 获取A2UI格式的组件树JSON（v0.9 updateComponents消息格式）
     * @return A2UI JSON字符串
     */
    virtual std::string getComponentTreeA2UI() = 0;
    
    /**
     * 根据ID获取组件句柄
     * @param id 组件ID（字符串形式）
     * @return 组件句柄
     */
    virtual JComponentHandle getComponentById(const std::string& id) = 0;
    
    /**
     * 开始录制会话
     * @return 会话ID
     */
    virtual std::string startRecording() = 0;
    
    /**
     * 停止录制会话
     * @return 录制的会话数据
     */
    virtual std::string stopRecording() = 0;
    
    /**
     * 回放会话
     * @param sessionData 会话数据
     */
    virtual void playback(const std::string& sessionData) = 0;
};

/**
 * 测试控制器类
 * 
 * 实现ITestController接口
 * 提供UI自动化测试的完整功能
 */
class JTestController : public ITestController {
public:
    /**
     * 构造函数
     * @param stateManager 状态管理器引用
     * @param dispatcher 事件分发器引用
     */
    explicit JTestController(JStateManager& stateManager, JEventDispatcher& dispatcher);
    
    /**
     * 获取组件树JSON
     * @return JSON字符串
     */
    std::string getComponentTreeJSON() override;
    
    /**
     * 获取组件属性
     * @param id 组件ID（字符串形式）
     * @param prop 属性名
     * @return 属性值的JSON字符串
     */
    std::string getProperty(const std::string& id, const std::string& prop) override;
    
    /**
     * 设置组件属性
     * @param id 组件ID（字符串形式）
     * @param prop 属性名
     * @param valueJson 属性值的JSON字符串
     * @return 是否成功
     */
    bool setProperty(const std::string& id, const std::string& prop, const std::string& valueJson) override;
    
    /**
     * 注入鼠标事件
     * @param type 事件类型
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void injectMouseEvent(const std::string& type, int x, int y, int button) override;
    
    /**
     * 注入键盘事件
     * @param type 事件类型
     * @param keyCode 键码
     * @param modifiers 修饰键
     */
    void injectKeyEvent(const std::string& type, int keyCode, int modifiers) override;
    
    /**
     * 注入文本输入
     * @param text 输入文本
     */
    void injectTextInput(const std::string& text) override;
    
    /**
     * 推进时间（用于动画和延迟测试）
     * @param ms 毫秒数
     */
    void advanceTime(int ms) override;
    
    /**
     * 拍摄快照
     * @return 快照JSON字符串
     */
    std::string takeSnapshot() override;
    
    /**
     * 等待条件满足
     * @param jsonPath JSON路径表达式
     * @param expectedValue 期望值
     * @param timeoutMs 超时时间（毫秒）
     * @return 条件是否满足
     */
    bool waitForCondition(const std::string& jsonPath, const std::string& expectedValue, int timeoutMs) override;
    
    /**
     * 获取事件日志
     * @return 事件日志列表
     */
    std::vector<std::string> getEventLog() override;
    
    /**
     * 根据ID获取组件句柄
     * @param id 组件ID（字符串形式）
     * @return 组件句柄
     */
    JComponentHandle getComponentById(const std::string& id) override;
    
    /**
     * 开始录制会话
     * @return 会话ID
     */
    std::string startRecording() override;
    
    /**
     * 停止录制会话
     * @return 录制的会话数据
     */
    std::string stopRecording() override;
    
    /**
     * 回放会话
     * @param sessionData 会话数据
     */
    void playback(const std::string& sessionData) override;
    
    /**
     * 获取A2UI格式的组件树JSON
     * @return A2UI JSON字符串
     */
    std::string getComponentTreeA2UI() override;
    
    /**
     * 设置A2UI生成器引用（用于导出A2UI格式）
     * @param generator A2UI生成器引用
     */
    void setA2UIGenerator(JA2UIGenerator* generator) { a2uiGenerator_ = generator; }
    
    /**
     * 设置当前时间（毫秒）
     * @param time 时间戳
     */
    void setCurrentTime(int64_t time) { 
        currentTime_ = time; 
        dispatcher_.setCurrentTime(time);
    }
    
    /**
     * 获取当前时间
     * @return 时间戳
     */
    int64_t getCurrentTime() const { return currentTime_; }
    
private:
    /**
     * 根据字符串ID查找组件
     * @param id 字符串ID
     * @return 组件句柄
     */
    JComponentHandle findComponentByIdString(const std::string& id);
    
    /**
     * 计算JSON路径表达式
     * @param jsonPath JSON路径
     * @return 路径值
     */
    std::string evaluateJSONPath(const std::string& jsonPath);
    
    /**
     * 生成会话ID
     * @return 唯一会话ID
     */
    std::string generateSessionId();
    
    JStateManager& stateManager_;       // 状态管理器引用
    JEventDispatcher& dispatcher_;     // 事件分发器引用
    JA2UIGenerator* a2uiGenerator_ = nullptr;  // A2UI生成器引用（可选）
    int64_t currentTime_ = 0;         // 当前时间
    std::chrono::steady_clock::time_point startTime_;  // 开始时间
};

}
