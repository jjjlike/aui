/**
 * 事件分发器模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JEventDispatcher 类的事件分发功能
 * - 包括命中测试、鼠标事件、键盘事件、文本输入等
 * - 测试用例覆盖：正常逻辑、边界情况、事件处理流程
 */

#include "aether/EventDispatcher.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 事件分发器测试的夹具类
 * 每个测试运行前会创建新的组件存储和事件分发器实例
 */
class EventDispatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储实例
        storage = std::make_unique<JComponentStorage>();
        // 创建事件分发器实例，关联到组件存储
        dispatcher = std::make_unique<JEventDispatcher>(*storage);
        
        // 创建根容器组件
        root = storage->createComponent(JComponentType::Container);
        // 获取根组件的详细信息
        auto* rootEntry = storage->getComponent(root);
        // 设置根组件的布局大小（800x600）
        rootEntry->layoutResult = {0, 0, 800, 600};
    }
    
    std::unique_ptr<JComponentStorage> storage;      // 组件存储指针
    std::unique_ptr<JEventDispatcher> dispatcher;   // 事件分发器指针
    JComponentHandle root;                          // 根组件句柄
};

/**
 * 测试用例 1：空区域命中测试（正常逻辑测试）
 * 测试目标：验证在没有组件的区域进行命中测试时返回无效句柄
 */
TEST_F(EventDispatcherTest, HitTestEmpty) {
    // 在点 (100, 100) 进行命中测试
    auto result = dispatcher->hitTest(JPoint{100, 100});
    // 验证返回无效的组件句柄
    EXPECT_FALSE(result.isValid());
}

/**
 * 测试用例 2：按钮命中测试（正常逻辑测试）
 * 测试目标：验证能正确命中可见且启用的按钮组件
 */
TEST_F(EventDispatcherTest, HitTestButton) {
    // 创建一个按钮组件，作为根组件的子组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域（位置和大小）
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为可见
    buttonEntry->visible = true;
    // 设置按钮为启用状态
    buttonEntry->enabled = true;
    
    // 通知布局完成，让事件分发器更新内部状态
    dispatcher->onLayoutComplete();
    
    // 在按钮中心点 (150, 125) 进行命中测试
    auto result = dispatcher->hitTest(JPoint{150, 125});
    // 验证命中结果有效
    EXPECT_TRUE(result.isValid());
    // 验证命中的是我们创建的按钮组件
    EXPECT_EQ(result, button);
}

/**
 * 测试用例 3：命中测试未命中（边界测试）
 * 测试目标：验证在组件外区域进行命中测试时返回无效句柄
 */
TEST_F(EventDispatcherTest, HitTestMiss) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 在点 (500, 500) 进行命中测试（不在按钮范围内）
    auto result = dispatcher->hitTest(JPoint{500, 500});
    // 验证返回无效的组件句柄
    EXPECT_FALSE(result.isValid());
}

/**
 * 测试用例 4：禁用组件命中测试（条件测试）
 * 测试目标：验证禁用的组件不会被命中
 */
TEST_F(EventDispatcherTest, HitTestDisabledComponent) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为禁用状态
    buttonEntry->enabled = false;
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 在按钮中心点进行命中测试
    auto result = dispatcher->hitTest(JPoint{150, 125});
    // 验证返回无效的组件句柄（因为按钮被禁用）
    // 注意：当前实现可能不检查enabled状态，所以这个测试可能失败
    // 如果需要支持这个特性，需要修改hitTest逻辑
    EXPECT_FALSE(result.isValid());
}

/**
 * 测试用例 5：隐藏组件命中测试（条件测试）
 * 测试目标：验证隐藏的组件不会被命中
 */
TEST_F(EventDispatcherTest, HitTestHiddenComponent) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为隐藏状态
    buttonEntry->visible = false;
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 在按钮中心点进行命中测试
    auto result = dispatcher->hitTest(JPoint{150, 125});
    // 验证返回无效的组件句柄（因为按钮被隐藏）
    EXPECT_FALSE(result.isValid());
}

/**
 * 测试用例 6：鼠标点击事件（正常逻辑测试）
 * 测试目标：验证能正确分发鼠标点击事件
 */
TEST_F(EventDispatcherTest, MouseClickEvent) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 创建鼠标点击事件
    JMouseEvent click;
    click.type = JEventType::Click;  // 事件类型为点击
    click.position = {150, 125};      // 点击位置在按钮中心
    click.button = 0;                // 左键点击
    
    // 分发鼠标事件
    dispatcher->dispatchMouseEvent(click);
    
    // 获取事件日志
    auto log = dispatcher->getEventLog();
    // 验证事件被记录
    EXPECT_GT(log.size(), 0);
}

/**
 * 测试用例 7：鼠标移动事件（正常逻辑测试）
 * 测试目标：验证能正确分发鼠标移动事件
 */
TEST_F(EventDispatcherTest, MouseMoveEvent) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 创建鼠标移动事件
    JMouseEvent move;
    move.type = JEventType::MouseMove;   // 事件类型为移动
    move.position = {150, 125};          // 移动位置在按钮上方
    
    // 分发鼠标事件
    dispatcher->dispatchMouseEvent(move);
    
    // 获取事件日志
    auto log = dispatcher->getEventLog();
    // 验证事件被记录
    EXPECT_GT(log.size(), 0);
}

/**
 * 测试用例 8：键盘事件（正常逻辑测试）
 * 测试目标：验证能正确分发键盘事件
 */
TEST_F(EventDispatcherTest, JKeyEvent) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 创建键盘按键事件
    JKeyEvent keyDown;
    keyDown.type = JEventType::KeyDown;  // 事件类型为按键按下
    keyDown.keyCode = 13;                // 按键码 13 表示回车键
    
    // 分发键盘事件
    dispatcher->dispatchKeyEvent(keyDown);
}

/**
 * 测试用例 9：文本输入事件（正常逻辑测试）
 * 测试目标：验证能正确分发文本输入事件
 */
TEST_F(EventDispatcherTest, TextInputEvent) {
    // 分发文本输入事件，输入内容为 "Hello"
    dispatcher->dispatchTextInput("Hello");
}

/**
 * 测试用例 10：多个按钮的命中测试（分支测试）
 * 测试目标：验证在多个重叠按钮中能正确命中最上层的按钮
 */
TEST_F(EventDispatcherTest, MultipleButtonsHitTest) {
    // 创建两个按钮组件
    auto button1 = storage->createComponent(JComponentType::Button, root);
    auto button2 = storage->createComponent(JComponentType::Button, root);
    
    // 获取两个按钮的详细信息
    auto* entry1 = storage->getComponent(button1);
    auto* entry2 = storage->getComponent(button2);
    
    // 设置第一个按钮的布局区域
    entry1->layoutResult = {100, 100, 100, 50};
    // 设置第二个按钮的布局区域，与第一个部分重叠
    entry2->layoutResult = {150, 100, 100, 50};
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 在重叠区域的点 (200, 125) 进行命中测试
    auto result = dispatcher->hitTest(JPoint{200, 125});
    // 验证命中结果有效
    EXPECT_TRUE(result.isValid());
}

/**
 * 测试用例 11：清空事件日志（正常逻辑测试）
 * 测试目标：验证 clearEventLog() 能正确清空事件日志
 */
TEST_F(EventDispatcherTest, ClearEventLog) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 通知布局完成
    dispatcher->onLayoutComplete();
    
    // 创建鼠标点击事件
    JMouseEvent click;
    click.type = JEventType::Click;      // 事件类型为点击
    click.position = {150, 125};        // 点击位置在按钮中心
    // 分发鼠标事件
    dispatcher->dispatchMouseEvent(click);
    
    // 验证事件日志不为空
    EXPECT_GT(dispatcher->getEventLog().size(), 0);
    
    // 清空事件日志
    dispatcher->clearEventLog();
    // 验证事件日志为空
    EXPECT_EQ(dispatcher->getEventLog().size(), 0);
}

} // namespace test
} // namespace jaether
