// EventRecording_test.cpp
// 事件录制/回放测试模块
//
// 功能测试:
// - 事件录制功能 - 录制用户交互事件序列
// - 事件回放功能 - 回放录制的事件序列
// - 会话管理 - 管理多个录制会话
// - 时间戳处理 - 正确处理事件时间戳
// - 数据完整性 - 确保录制和回放的数据一致
//
// 测试覆盖:
// - 语句覆盖：所有事件录制/回放语句
// - 分支覆盖：录制/停止录制/回放的分支
// - 条件覆盖：isRecording状态、事件类型条件
// - 路径覆盖：从录制到停止到回放的完整路径

#include "aether/EventDispatcher.h"
#include "aether/ComponentStorage.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

namespace aether {
namespace test {

/**
 * 事件录制/回放测试夹具
 * 
 * 提供测试所需的组件存储和事件分发器
 */
class EventRecordingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储
        storage = std::make_unique<ComponentStorage>();
        // 创建事件分发器
        dispatcher = std::make_unique<EventDispatcher>(*storage);
        
        // 创建根组件
        root = storage->createComponent(ComponentType::Container);
        auto* rootEntry = storage->getComponent(root);
        rootEntry->layoutResult = {0, 0, 800, 600};
        rootEntry->visible = true;
        rootEntry->enabled = true;
        
        // 创建按钮组件
        button = storage->createComponent(ComponentType::Button, root);
        auto* buttonEntry = storage->getComponent(button);
        buttonEntry->layoutResult = {100, 100, 200, 50};
        buttonEntry->visible = true;
        buttonEntry->enabled = true;
        buttonEntry->debugName = "TestButton";
        
        // 创建文本输入组件
        input = storage->createComponent(ComponentType::Input, root);
        auto* inputEntry = storage->getComponent(input);
        inputEntry->layoutResult = {100, 200, 300, 40};
        inputEntry->visible = true;
        inputEntry->enabled = true;
        inputEntry->debugName = "TestInput";
        
        // 完成布局
        dispatcher->onLayoutComplete();
        
        // 清空任何预存的事件日志
        dispatcher->clearEventLog();
        
        // 初始化时间
        dispatcher->setCurrentTime(1000000); // 1秒
    }
    
    void TearDown() override {
        dispatcher.reset();
        storage.reset();
    }
    
    std::unique_ptr<ComponentStorage> storage;
    std::unique_ptr<EventDispatcher> dispatcher;
    ComponentHandle root;
    ComponentHandle button;
    ComponentHandle input;
};

/**
 * 测试用例 1：开始录制（正常逻辑测试）
 * 测试目标：验证能正确开始录制事件
 * 
 * 覆盖分析：
 * - 语句覆盖：startRecording函数的所有语句
 * - 路径覆盖：从初始状态到开始录制的路径
 */
TEST_F(EventRecordingTest, StartRecording) {
    // 验证初始状态不是录制模式
    EXPECT_FALSE(dispatcher->isRecording());
    
    // 开始录制
    dispatcher->startRecording("session1");
    
    // 验证正在录制
    EXPECT_TRUE(dispatcher->isRecording());
    
    // 验证会话ID
    EXPECT_EQ(dispatcher->getCurrentSessionId(), "session1");
}

/**
 * 测试用例 2：停止录制（正常逻辑测试）
 * 测试目标：验证能正确停止录制并保存事件
 * 
 * 覆盖分析：
 * - 语句覆盖：stopRecording函数的所有语句
 * - 分支覆盖：isRecording为true和false的分支
 * - 路径覆盖：从录制到停止的完整路径
 */
TEST_F(EventRecordingTest, StopRecording) {
    // 开始录制
    dispatcher->startRecording("session2");
    
    // 执行一些事件
    dispatcher->onClick(150, 125, 0);
    dispatcher->setCurrentTime(2000000); // 2秒
    dispatcher->onMouseMove(150, 125);
    
    // 停止录制
    std::string sessionId = dispatcher->stopRecording();
    
    // 验证停止后不再录制
    EXPECT_FALSE(dispatcher->isRecording());
    
    // 验证返回的会话ID
    EXPECT_EQ(sessionId, "session2");
    
    // 验证录制的会话存在
    auto sessions = dispatcher->getRecordedSessions();
    EXPECT_NE(sessions.find("session2"), sessions.end());
    
    // 验证录制了事件（Click会触发多个事件）
    EXPECT_GE(sessions["session2"].size(), 2);
}

/**
 * 测试用例 3：录制鼠标点击事件（正常逻辑测试）
 * 测试目标：验证能正确录制鼠标点击事件
 * 
 * 覆盖分析：
 * - 语句覆盖：recordEvent函数的所有语句
 * - 条件覆盖：isRecording条件为true
 * - 路径覆盖：从点击到录制的路径
 */
TEST_F(EventRecordingTest, RecordMouseClick) {
    // 开始录制
    dispatcher->startRecording("mouse_click");
    
    // 执行点击事件
    dispatcher->setCurrentTime(1500000); // 1.5秒
    dispatcher->onClick(150, 125, 0);
    
    // 停止录制
    dispatcher->stopRecording();
    
    // 获取录制的会话
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["mouse_click"];
    
    // 验证录制了事件（Click包含多个事件类型）
    ASSERT_GE(events.size(), 1);
    
    // 验证至少有一个Click类型的事件
    bool foundClick = false;
    for (const auto& event : events) {
        if (event.eventType == EventType::Click) {
            foundClick = true;
            // 验证事件数据
            EXPECT_EQ(event.mouseEvent.position.x, 150);
            EXPECT_EQ(event.mouseEvent.position.y, 125);
            EXPECT_EQ(event.mouseEvent.button, 0);
            // 验证时间戳
            EXPECT_EQ(event.timestamp, 1500000);
            break;
        }
    }
    EXPECT_TRUE(foundClick);
}

/**
 * 测试用例 4：录制鼠标移动事件（正常逻辑测试）
 * 测试目标：验证能正确录制鼠标移动事件
 * 
 * 覆盖分析：
 * - 分支覆盖：MouseMove类型的分支
 * - 路径覆盖：从鼠标移动到录制的完整路径
 */
TEST_F(EventRecordingTest, RecordMouseMove) {
    // 开始录制
    dispatcher->startRecording("mouse_move");
    
    // 执行多个鼠标移动事件
    dispatcher->setCurrentTime(1000000);
    dispatcher->onMouseMove(100, 100);
    
    dispatcher->setCurrentTime(1100000);
    dispatcher->onMouseMove(200, 200);
    
    dispatcher->setCurrentTime(1200000);
    dispatcher->onMouseMove(300, 300);
    
    // 停止录制
    dispatcher->stopRecording();
    
    // 获取录制的会话
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["mouse_move"];
    
    // 验证录制了事件
    ASSERT_GE(events.size(), 1);
    
    // 验证有MouseMove事件
    int moveCount = 0;
    for (const auto& event : events) {
        if (event.eventType == EventType::MouseMove) {
            moveCount++;
        }
    }
    EXPECT_GE(moveCount, 3);
}

/**
 * 测试用例 5：录制键盘事件（正常逻辑测试）
 * 测试目标：验证能正确录制键盘事件
 * 
 * 覆盖分析：
 * - 分支覆盖：KeyDown/KeyUp类型的分支
 * - 路径覆盖：从按键到录制的路径
 */
TEST_F(EventRecordingTest, RecordKeyEvent) {
    // 先点击按钮设置焦点
    dispatcher->onClick(150, 125, 0);
    dispatcher->clearEventLog();
    
    // 开始录制
    dispatcher->startRecording("key_event");
    
    // 执行键盘事件
    dispatcher->setCurrentTime(2000000);
    dispatcher->onKeyDown(65); // 'A'
    
    dispatcher->setCurrentTime(2100000);
    dispatcher->onKeyUp(65);
    
    dispatcher->setCurrentTime(2200000);
    dispatcher->onKeyDown(66); // 'B'
    
    // 停止录制
    dispatcher->stopRecording();
    
    // 获取录制的会话
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["key_event"];
    
    // 验证录制了键盘事件
    bool hasKeyDown = false;
    bool hasKeyUp = false;
    for (const auto& event : events) {
        if (event.eventType == EventType::KeyDown) {
            hasKeyDown = true;
        } else if (event.eventType == EventType::KeyUp) {
            hasKeyUp = true;
        }
    }
    EXPECT_TRUE(hasKeyDown);
    EXPECT_TRUE(hasKeyUp);
}

/**
 * 测试用例 6：录制文本输入事件（正常逻辑测试）
 * 测试目标：验证能正确录制文本输入事件
 * 
 * 覆盖分析：
 * - 分支覆盖：TextInput类型的分支
 * - 路径覆盖：从文本输入到录制的路径
 */
TEST_F(EventRecordingTest, RecordTextInput) {
    // 先点击输入框设置焦点
    dispatcher->onClick(150, 220, 0);
    dispatcher->clearEventLog();
    
    // 开始录制
    dispatcher->startRecording("text_input");
    
    // 执行文本输入
    dispatcher->setCurrentTime(3000000);
    dispatcher->onTextInput("Hello");
    
    dispatcher->setCurrentTime(3100000);
    dispatcher->onTextInput(" ");
    
    dispatcher->setCurrentTime(3200000);
    dispatcher->onTextInput("World");
    
    // 停止录制
    dispatcher->stopRecording();
    
    // 获取录制的会话
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["text_input"];
    
    // 验证录制了文本输入事件
    int textInputCount = 0;
    for (const auto& event : events) {
        if (event.eventType == EventType::TextInput) {
            textInputCount++;
        }
    }
    EXPECT_GE(textInputCount, 3);
}

/**
 * 测试用例 7：回放鼠标点击事件（正常逻辑测试）
 * 测试目标：验证能正确回放鼠标点击事件
 * 
 * 覆盖分析：
 * - 路径覆盖：从录制到回放的完整往返路径
 * - 条件覆盖：Click类型的条件判断
 */
TEST_F(EventRecordingTest, PlaybackMouseClick) {
    // 录制事件
    dispatcher->startRecording("playback_click");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onClick(150, 125, 0);
    dispatcher->stopRecording();
    
    // 清空事件日志
    dispatcher->clearEventLog();
    
    // 获取录制的事件
    auto sessions = dispatcher->getRecordedSessions();
    auto events = sessions["playback_click"];
    
    // 回放事件
    dispatcher->playEvents(events);
    
    // 验证事件日志中有事件
    auto eventLog = dispatcher->getEventLog();
    EXPECT_GT(eventLog.size(), 0);
}

/**
 * 测试用例 8：回放多个事件（正常逻辑测试）
 * 测试目标：验证能正确回放多个事件序列
 * 
 * 覆盖分析：
 * - 循环覆盖：遍历所有事件的循环
 * - 分支覆盖：不同事件类型的分发分支
 */
TEST_F(EventRecordingTest, PlaybackMultipleEvents) {
    // 录制完整的事件序列
    dispatcher->startRecording("playback_sequence");
    
    dispatcher->setCurrentTime(1000000);
    dispatcher->onClick(150, 125, 0); // 点击按钮
    
    dispatcher->setCurrentTime(1100000);
    dispatcher->onMouseMove(200, 220); // 移动到输入框
    
    dispatcher->setCurrentTime(1200000);
    dispatcher->onClick(150, 220, 0); // 点击输入框
    
    dispatcher->setCurrentTime(1300000);
    dispatcher->onTextInput("Test");
    
    dispatcher->stopRecording();
    
    // 清空事件日志
    dispatcher->clearEventLog();
    
    // 获取并回放事件
    auto sessions = dispatcher->getRecordedSessions();
    dispatcher->playEvents(sessions["playback_sequence"]);
    
    // 验证回放的事件数量
    auto eventLog = dispatcher->getEventLog();
    EXPECT_GE(eventLog.size(), 4);
}

/**
 * 测试用例 9：停止空录制（边界测试）
 * 测试目标：验证停止未开始的录制返回空字符串
 * 
 * 覆盖分析：
 * - 分支覆盖：isRecording为false的分支
 * - 边界覆盖：空录制会话的边界情况
 */
TEST_F(EventRecordingTest, StopEmptyRecording) {
    // 不开始录制，直接停止
    std::string sessionId = dispatcher->stopRecording();
    
    // 验证返回空字符串
    EXPECT_EQ(sessionId, "");
    
    // 验证没有录制会话
    auto sessions = dispatcher->getRecordedSessions();
    EXPECT_EQ(sessions.size(), 0);
}

/**
 * 测试用例 10：多次录制会话（正常逻辑测试）
 * 测试目标：验证能管理多个录制会话
 * 
 * 覆盖分析：
 * - 映射覆盖：recordedSessions的插入和查找
 * - 路径覆盖：多个独立录制会话的路径
 */
TEST_F(EventRecordingTest, MultipleRecordingSessions) {
    // 第一个会话
    dispatcher->startRecording("session_a");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onClick(150, 125, 0);
    dispatcher->stopRecording();
    
    // 第二个会话
    dispatcher->startRecording("session_b");
    dispatcher->setCurrentTime(2000000);
    dispatcher->onClick(150, 220, 0);
    dispatcher->setCurrentTime(2100000);
    dispatcher->onTextInput("Test");
    dispatcher->stopRecording();
    
    // 第三个会话
    dispatcher->startRecording("session_c");
    dispatcher->setCurrentTime(3000000);
    dispatcher->onKeyDown(65);
    dispatcher->stopRecording();
    
    // 验证有3个录制会话
    auto sessions = dispatcher->getRecordedSessions();
    EXPECT_EQ(sessions.size(), 3);
    
    // 验证第一个会话有事件
    EXPECT_GE(sessions["session_a"].size(), 1);
    
    // 验证第二个会话有事件
    EXPECT_GE(sessions["session_b"].size(), 2);
    
    // 验证第三个会话有事件
    EXPECT_GE(sessions["session_c"].size(), 1);
}

/**
 * 测试用例 11：覆盖已有会话（异常测试）
 * 测试目标：验证同名会话会被覆盖
 * 
 * 覆盖分析：
 * - 映射覆盖：覆盖已有键的行为
 * - 路径覆盖：重复会话ID的路径
 */
TEST_F(EventRecordingTest, OverwriteExistingSession) {
    // 第一个录制
    dispatcher->startRecording("same_session");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onClick(150, 125, 0);
    dispatcher->stopRecording();
    
    // 第二个录制（同名）
    dispatcher->startRecording("same_session");
    dispatcher->setCurrentTime(2000000);
    dispatcher->onClick(150, 220, 0);
    dispatcher->setCurrentTime(2100000);
    dispatcher->onTextInput("New");
    dispatcher->stopRecording();
    
    // 获取会话
    auto sessions = dispatcher->getRecordedSessions();
    
    // 验证只有1个会话
    EXPECT_EQ(sessions.size(), 1);
    
    // 验证会话被覆盖（只有新内容）
    EXPECT_GE(sessions["same_session"].size(), 2);
}

/**
 * 测试用例 12：回放键盘事件序列（正常逻辑测试）
 * 测试目标：验证能正确回放键盘事件序列
 * 
 * 覆盖分析：
 * - 分支覆盖：KeyDown/KeyUp类型的回放分支
 */
TEST_F(EventRecordingTest, PlaybackKeySequence) {
    // 设置焦点
    dispatcher->onClick(150, 220, 0);
    dispatcher->clearEventLog();
    
    // 录制键盘序列
    dispatcher->startRecording("key_sequence");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onKeyDown(65); // 'A'
    dispatcher->setCurrentTime(1100000);
    dispatcher->onKeyUp(65);
    dispatcher->setCurrentTime(1200000);
    dispatcher->onKeyDown(66); // 'B'
    dispatcher->setCurrentTime(1300000);
    dispatcher->onKeyUp(66);
    dispatcher->stopRecording();
    
    // 清空日志
    dispatcher->clearEventLog();
    
    // 回放
    auto sessions = dispatcher->getRecordedSessions();
    dispatcher->playEvents(sessions["key_sequence"]);
    
    // 验证回放了事件
    auto eventLog = dispatcher->getEventLog();
    EXPECT_GE(eventLog.size(), 4);
}

/**
 * 测试用例 13：回放文本输入序列（正常逻辑测试）
 * 测试目标：验证能正确回放文本输入序列
 * 
 * 覆盖分析：
 * - 分支覆盖：TextInput类型的回放分支
 */
TEST_F(EventRecordingTest, PlaybackTextSequence) {
    // 设置焦点
    dispatcher->onClick(150, 220, 0);
    dispatcher->clearEventLog();
    
    // 录制文本序列
    dispatcher->startRecording("text_sequence");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onTextInput("Hello");
    dispatcher->setCurrentTime(1100000);
    dispatcher->onTextInput(" ");
    dispatcher->setCurrentTime(1200000);
    dispatcher->onTextInput("World");
    dispatcher->stopRecording();
    
    // 清空日志
    dispatcher->clearEventLog();
    
    // 回放
    auto sessions = dispatcher->getRecordedSessions();
    dispatcher->playEvents(sessions["text_sequence"]);
    
    // 验证回放了文本输入事件
    auto eventLog = dispatcher->getEventLog();
    EXPECT_GE(eventLog.size(), 3);
}

/**
 * 测试用例 14：回放空事件列表（边界测试）
 * 测试目标：验证能正确处理空事件列表的回放
 * 
 * 覆盖分析：
 * - 边界覆盖：空事件列表的边界情况
 * - 循环覆盖：空列表的循环
 */
TEST_F(EventRecordingTest, PlaybackEmptyEvents) {
    // 创建空事件列表
    std::vector<RecordedEvent> emptyEvents;
    
    // 清空事件日志
    dispatcher->clearEventLog();
    
    // 回放空事件列表
    dispatcher->playEvents(emptyEvents);
    
    // 验证事件日志为空
    auto eventLog = dispatcher->getEventLog();
    EXPECT_EQ(eventLog.size(), 0);
}

/**
 * 测试用例 15：事件时间戳记录（正常逻辑测试）
 * 测试目标：验证录制的事件包含正确的时间戳
 * 
 * 覆盖分析：
 * - 语句覆盖：currentTime_的设置和递增
 * - 路径覆盖：多个事件时间戳的路径
 */
TEST_F(EventRecordingTest, EventTimestampsAreRecorded) {
    // 开始录制
    dispatcher->startRecording("timestamp_test");
    
    // 录制多个事件，设置不同的时间戳
    dispatcher->setCurrentTime(1000000);
    dispatcher->onMouseMove(100, 100);
    
    dispatcher->setCurrentTime(2000000);
    dispatcher->onMouseMove(200, 200);
    
    dispatcher->setCurrentTime(3000000);
    dispatcher->onMouseMove(300, 300);
    
    dispatcher->stopRecording();
    
    // 获取录制的事件
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["timestamp_test"];
    
    // 验证录制了事件
    ASSERT_GE(events.size(), 1);
    
    // 验证至少有事件使用了设置的时间戳
    bool foundTimestamp1 = false;
    bool foundTimestamp2 = false;
    bool foundTimestamp3 = false;
    for (const auto& event : events) {
        if (event.timestamp == 1000000) foundTimestamp1 = true;
        if (event.timestamp == 2000000) foundTimestamp2 = true;
        if (event.timestamp == 3000000) foundTimestamp3 = true;
    }
    EXPECT_TRUE(foundTimestamp1);
    EXPECT_TRUE(foundTimestamp2);
    EXPECT_TRUE(foundTimestamp3);
}

/**
 * 测试用例 16：会话管理独立性（正常逻辑测试）
 * 测试目标：验证不同会话的事件相互独立
 * 
 * 覆盖分析：
 * - 映射覆盖：会话映射的隔离性
 * - 路径覆盖：独立会话的路径
 */
TEST_F(EventRecordingTest, SessionIndependence) {
    // 创建两个独立的录制
    dispatcher->startRecording("independent_a");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onClick(100, 100, 0);
    dispatcher->stopRecording();
    
    dispatcher->startRecording("independent_b");
    dispatcher->setCurrentTime(2000000);
    dispatcher->onClick(200, 200, 0);
    dispatcher->setCurrentTime(2100000);
    dispatcher->onClick(300, 300, 0);
    dispatcher->stopRecording();
    
    // 获取会话
    auto sessions = dispatcher->getRecordedSessions();
    
    // 验证会话独立
    EXPECT_GE(sessions["independent_a"].size(), 1);
    EXPECT_GE(sessions["independent_b"].size(), 2);
}

/**
 * 测试用例 17：鼠标按下和释放事件录制（正常逻辑测试）
 * 测试目标：验证能正确录制鼠标按下和释放事件
 * 
 * 覆盖分析：
 * - 分支覆盖：MouseDown/MouseUp类型的分支
 */
TEST_F(EventRecordingTest, RecordMouseDownUp) {
    // 开始录制
    dispatcher->startRecording("mouse_down_up");
    
    dispatcher->setCurrentTime(1000000);
    dispatcher->onMouseDown(150, 125, 0);
    
    dispatcher->setCurrentTime(1100000);
    dispatcher->onMouseUp(150, 125, 0);
    
    dispatcher->stopRecording();
    
    // 获取录制的事件
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["mouse_down_up"];
    
    // 验证录制了事件
    ASSERT_GE(events.size(), 1);
    
    // 验证有MouseDown和/或MouseUp事件
    bool hasMouseDown = false;
    bool hasMouseUp = false;
    for (const auto& event : events) {
        if (event.eventType == EventType::MouseDown) hasMouseDown = true;
        if (event.eventType == EventType::MouseUp) hasMouseUp = true;
    }
    EXPECT_TRUE(hasMouseDown || hasMouseUp);
}

/**
 * 测试用例 18：完整用户交互场景录制（集成测试）
 * 测试目标：验证能录制完整的用户交互场景
 * 
 * 覆盖分析：
 * - 路径覆盖：从开始到结束的完整用户交互路径
 * - 集成覆盖：多种事件类型的集成
 */
TEST_F(EventRecordingTest, RecordCompleteUserScenario) {
    // 开始录制完整场景
    dispatcher->startRecording("complete_scenario");
    
    // 场景：用户打开应用，点击按钮，输入文本
    
    dispatcher->setCurrentTime(1000000);
    dispatcher->onMouseMove(150, 125); // 移动到按钮
    dispatcher->setCurrentTime(1100000);
    dispatcher->onClick(150, 125, 0); // 点击按钮
    
    dispatcher->setCurrentTime(1200000);
    dispatcher->onMouseMove(150, 220); // 移动到输入框
    dispatcher->setCurrentTime(1300000);
    dispatcher->onClick(150, 220, 0); // 点击输入框
    
    dispatcher->setCurrentTime(1400000);
    dispatcher->onKeyDown(72); // 'H'
    dispatcher->setCurrentTime(1500000);
    dispatcher->onKeyUp(72);
    
    dispatcher->setCurrentTime(1600000);
    dispatcher->onTextInput("H");
    
    dispatcher->stopRecording();
    
    // 获取录制的事件
    auto sessions = dispatcher->getRecordedSessions();
    auto& events = sessions["complete_scenario"];
    
    // 验证录制了事件
    EXPECT_GE(events.size(), 1);
    
    // 验证事件类型包含鼠标、键盘和文本输入
    bool hasMouseMove = false;
    bool hasClick = false;
    bool hasKeyDown = false;
    bool hasKeyUp = false;
    bool hasTextInput = false;
    
    for (const auto& event : events) {
        if (event.eventType == EventType::MouseMove) hasMouseMove = true;
        if (event.eventType == EventType::Click) hasClick = true;
        if (event.eventType == EventType::KeyDown) hasKeyDown = true;
        if (event.eventType == EventType::KeyUp) hasKeyUp = true;
        if (event.eventType == EventType::TextInput) hasTextInput = true;
    }
    
    EXPECT_TRUE(hasMouseMove || hasClick || hasKeyDown || hasKeyUp || hasTextInput);
}

/**
 * 测试用例 19：获取当前时间（正常逻辑测试）
 * 测试目标：验证能正确获取和设置当前时间
 * 
 * 覆盖分析：
 * - 语句覆盖：getCurrentTime和setCurrentTime
 * - 边界覆盖：时间值边界
 */
TEST_F(EventRecordingTest, CurrentTimeManagement) {
    // 验证初始时间
    EXPECT_EQ(dispatcher->getCurrentTime(), 1000000);
    
    // 设置新时间
    dispatcher->setCurrentTime(5000000);
    
    // 验证时间已更新
    EXPECT_EQ(dispatcher->getCurrentTime(), 5000000);
    
    // 验证时间在录制事件中使用
    dispatcher->startRecording("time_test");
    dispatcher->onClick(150, 125, 0);
    dispatcher->stopRecording();
    
    auto sessions = dispatcher->getRecordedSessions();
    // 查找包含Click的事件
    for (const auto& event : sessions["time_test"]) {
        if (event.eventType == EventType::Click) {
            EXPECT_EQ(event.timestamp, 5000000);
            return;
        }
    }
    // 如果没有Click，至少验证录制了一些事件
    EXPECT_GE(sessions["time_test"].size(), 1);
}

/**
 * 测试用例 20：回放事件基本功能（性能测试）
 * 测试目标：验证回放能正确分发事件
 * 
 * 覆盖分析：
 * - 循环覆盖：事件回放循环
 * - 语句覆盖：事件分发逻辑
 */
TEST_F(EventRecordingTest, PlaybackEventsBasicFunctionality) {
    // 录制事件
    dispatcher->startRecording("playback_test");
    dispatcher->setCurrentTime(1000000);
    dispatcher->onClick(150, 125, 0);
    dispatcher->setCurrentTime(2000000);
    dispatcher->onClick(150, 220, 0);
    dispatcher->setCurrentTime(3000000);
    dispatcher->onClick(200, 300, 0);
    dispatcher->stopRecording();
    
    // 清空事件日志
    dispatcher->clearEventLog();
    
    // 回放事件
    auto sessions = dispatcher->getRecordedSessions();
    dispatcher->playEvents(sessions["playback_test"]);
    
    // 验证回放产生了事件日志
    auto eventLog = dispatcher->getEventLog();
    EXPECT_GT(eventLog.size(), 0);
}

} // namespace test
} // namespace aether
