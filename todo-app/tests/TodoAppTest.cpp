// TodoAppTest.cpp
// 待办列表应用测试
//
// 功能测试:
// - 应用初始化
// - 添加待办项
// - 标记完成/取消
// - 删除待办项
// - 统计信息更新

#include "TodoApp.h"
#include "aether/LogicLayer.h"
#include "aether/EventDispatcher.h"
#include <gtest/gtest.h>
#include <string>

namespace jaether {
namespace test {

/**
 * 待办应用测试夹具
 */
class TodoAppTest : public ::testing::Test {
protected:
    void SetUp() override {
        logicLayer_ = std::make_unique<JLogicLayer>();
        todoApp_ = std::make_unique<JTodoApp>(*logicLayer_);
    }
    
    void TearDown() override {
        todoApp_.reset();
        logicLayer_.reset();
    }
    
    std::unique_ptr<JLogicLayer> logicLayer_;
    std::unique_ptr<JTodoApp> todoApp_;
};

/**
 * 测试用例 1: 应用初始化
 * 测试目标: 验证应用能正常初始化，UI组件创建正确
 * 
 * 覆盖分析:
 * - 语句覆盖: initialize方法所有语句
 * - 路径覆盖: 初始化流程
 */
TEST_F(TodoAppTest, ApplicationInitialization) {
    // 初始化应用
    todoApp_->initialize();
    
    // 验证统计信息
    EXPECT_EQ(todoApp_->getTotalCount(), 0);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    EXPECT_EQ(todoApp_->getPendingCount(), 0);
}

/**
 * 测试用例 2: 添加单个待办项
 * 测试目标: 验证添加单个待办项功能正常
 * 
 * 覆盖分析:
 * - 语句覆盖: addTodo方法
 * - 路径覆盖: 添加单个待办项的流程
 */
TEST_F(TodoAppTest, AddSingleTodoItem) {
    todoApp_->initialize();
    
    // 添加待办项
    todoApp_->addTodo("购买牛奶");
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    EXPECT_EQ(todoApp_->getPendingCount(), 1);
}

/**
 * 测试用例 3: 添加多个待办项
 * 测试目标: 验证能正确添加多个待办项
 * 
 * 覆盖分析:
 * - 循环覆盖: 多个添加操作
 * - 边界覆盖: 多个待办项的边界
 */
TEST_F(TodoAppTest, AddMultipleTodoItems) {
    todoApp_->initialize();
    
    // 添加多个待办项
    todoApp_->addTodo("购买牛奶");
    todoApp_->addTodo("复习功课");
    todoApp_->addTodo("锻炼身体");
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 3);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    EXPECT_EQ(todoApp_->getPendingCount(), 3);
}

/**
 * 测试用例 4: 添加空待办项
 * 测试目标: 验证空字符串不被添加
 * 
 * 覆盖分析:
 * - 边界覆盖: 空输入的边界情况
 * - 分支覆盖: text.empty()条件
 */
TEST_F(TodoAppTest, AddEmptyTodoItem) {
    todoApp_->initialize();
    
    // 添加空待办项
    todoApp_->addTodo("");
    
    // 验证没有被添加
    EXPECT_EQ(todoApp_->getTotalCount(), 0);
}

/**
 * 测试用例 5: 标记单个待办项为完成
 * 测试目标: 验证能正确标记待办项为完成
 * 
 * 覆盖分析:
 * - 语句覆盖: toggleTodo方法
 * - 条件覆盖: !todo.completed条件
 */
TEST_F(TodoAppTest, MarkSingleTodoComplete) {
    todoApp_->initialize();
    
    todoApp_->addTodo("购买牛奶");
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    
    // 标记完成
    todoApp_->toggleTodo(0);
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
    EXPECT_EQ(todoApp_->getCompletedCount(), 1);
    EXPECT_EQ(todoApp_->getPendingCount(), 0);
}

/**
 * 测试用例 6: 取消完成标记
 * 测试目标: 验证能取消待办项的完成状态
 * 
 * 覆盖分析:
 * - 路径覆盖: 从完成到未完成的路径
 */
TEST_F(TodoAppTest, UnmarkTodoComplete) {
    todoApp_->initialize();
    
    todoApp_->addTodo("购买牛奶");
    todoApp_->toggleTodo(0);
    EXPECT_EQ(todoApp_->getCompletedCount(), 1);
    
    // 取消完成
    todoApp_->toggleTodo(0);
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    EXPECT_EQ(todoApp_->getPendingCount(), 1);
}

/**
 * 测试用例 7: 标记多个待办项为完成
 * 测试目标: 验证能正确标记多个待办项
 * 
 * 覆盖分析:
 * - 循环覆盖: 多个待办项的标记
 */
TEST_F(TodoAppTest, MarkMultipleTodosComplete) {
    todoApp_->initialize();
    
    todoApp_->addTodo("购买牛奶");
    todoApp_->addTodo("复习功课");
    todoApp_->addTodo("锻炼身体");
    
    // 标记第0和第2个为完成
    todoApp_->toggleTodo(0);
    todoApp_->toggleTodo(2);
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 3);
    EXPECT_EQ(todoApp_->getCompletedCount(), 2);
    EXPECT_EQ(todoApp_->getPendingCount(), 1);
}

/**
 * 测试用例 8: 删除单个待办项
 * 测试目标: 验证能正确删除单个待办项
 * 
 * 覆盖分析:
 * - 语句覆盖: deleteTodo方法
 * - 路径覆盖: 删除待办项的流程
 */
TEST_F(TodoAppTest, DeleteSingleTodoItem) {
    todoApp_->initialize();
    
    todoApp_->addTodo("购买牛奶");
    todoApp_->addTodo("复习功课");
    
    EXPECT_EQ(todoApp_->getTotalCount(), 2);
    
    // 删除第一个
    todoApp_->deleteTodo(0);
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
}

/**
 * 测试用例 9: 删除无效索引的待办项
 * 测试目标: 验证无效索引被正确处理
 * 
 * 覆盖分析:
 * - 边界覆盖: 索引越界的边界情况
 * - 分支覆盖: index >= todos_.size()条件
 */
TEST_F(TodoAppTest, DeleteTodoWithInvalidIndex) {
    todoApp_->initialize();
    
    todoApp_->addTodo("购买牛奶");
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
    
    // 删除无效索引
    todoApp_->deleteTodo(static_cast<size_t>(100));
    todoApp_->deleteTodo(static_cast<size_t>(-1));
    
    // 验证没有发生变化
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
}

/**
 * 测试用例 10: 统计信息正确更新
 * 测试目标: 验证统计信息能正确反映当前状态
 * 
 * 覆盖分析:
 * - 语句覆盖: getCompletedCount方法
 * - 路径覆盖: 遍历所有待办项计算统计
 */
TEST_F(TodoAppTest, StatisticsUpdateCorrectly) {
    todoApp_->initialize();
    
    // 初始状态
    EXPECT_EQ(todoApp_->getTotalCount(), 0);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    EXPECT_EQ(todoApp_->getPendingCount(), 0);
    
    // 添加待办项
    todoApp_->addTodo("任务1");
    todoApp_->addTodo("任务2");
    todoApp_->addTodo("任务3");
    
    EXPECT_EQ(todoApp_->getTotalCount(), 3);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
    EXPECT_EQ(todoApp_->getPendingCount(), 3);
    
    // 标记部分完成
    todoApp_->toggleTodo(1);
    
    EXPECT_EQ(todoApp_->getTotalCount(), 3);
    EXPECT_EQ(todoApp_->getCompletedCount(), 1);
    EXPECT_EQ(todoApp_->getPendingCount(), 2);
    
    // 删除一个
    todoApp_->deleteTodo(0);
    
    EXPECT_EQ(todoApp_->getTotalCount(), 2);
    EXPECT_EQ(todoApp_->getCompletedCount(), 1);
    EXPECT_EQ(todoApp_->getPendingCount(), 1);
}

/**
 * 测试用例 11: 切换无效索引的待办项
 * 测试目标: 验证无效索引的切换被正确处理
 * 
 * 覆盖分析:
 * - 边界覆盖: 索引越界的边界情况
 */
TEST_F(TodoAppTest, ToggleTodoWithInvalidIndex) {
    todoApp_->initialize();
    
    todoApp_->addTodo("购买牛奶");
    
    // 切换无效索引
    todoApp_->toggleTodo(static_cast<size_t>(100));
    todoApp_->toggleTodo(static_cast<size_t>(-1));
    
    // 验证状态没有改变
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
}

/**
 * 测试用例 12: 完整用户流程测试
 * 测试目标: 验证完整的用户使用流程
 * 
 * 覆盖分析:
 * - 集成覆盖: 所有功能的集成使用
 * - 路径覆盖: 从添加到删除的完整流程
 */
TEST_F(TodoAppTest, CompleteUserWorkflow) {
    todoApp_->initialize();
    
    // 添加多个待办项
    todoApp_->addTodo("起床");
    todoApp_->addTodo("洗漱");
    todoApp_->addTodo("吃早餐");
    todoApp_->addTodo("上班");
    todoApp_->addTodo("回家");
    
    EXPECT_EQ(todoApp_->getTotalCount(), 5);
    EXPECT_EQ(todoApp_->getPendingCount(), 5);
    
    // 完成部分任务
    todoApp_->toggleTodo(0);
    todoApp_->toggleTodo(1);
    
    EXPECT_EQ(todoApp_->getCompletedCount(), 2);
    EXPECT_EQ(todoApp_->getPendingCount(), 3);
    
    // 添加新任务
    todoApp_->addTodo("睡觉");
    
    EXPECT_EQ(todoApp_->getTotalCount(), 6);
    
    // 删除一个任务
    todoApp_->deleteTodo(3);
    
    EXPECT_EQ(todoApp_->getTotalCount(), 5);
    
    // 验证最终状态
    EXPECT_EQ(todoApp_->getCompletedCount(), 2);
    EXPECT_EQ(todoApp_->getPendingCount(), 3);
}

/**
 * 测试用例 13: 标记所有待办项为完成
 * 测试目标: 验证能标记所有待办项为完成
 * 
 * 覆盖分析:
 * - 循环覆盖: 所有待办项都被标记
 */
TEST_F(TodoAppTest, MarkAllTodosComplete) {
    todoApp_->initialize();
    
    todoApp_->addTodo("任务1");
    todoApp_->addTodo("任务2");
    todoApp_->addTodo("任务3");
    
    EXPECT_EQ(todoApp_->getPendingCount(), 3);
    
    // 标记所有为完成
    todoApp_->toggleTodo(0);
    todoApp_->toggleTodo(1);
    todoApp_->toggleTodo(2);
    
    // 验证
    EXPECT_EQ(todoApp_->getCompletedCount(), 3);
    EXPECT_EQ(todoApp_->getPendingCount(), 0);
}

/**
 * 测试用例 14: 删除所有待办项
 * 测试目标: 验证能删除所有待办项
 * 
 * 覆盖分析:
 * - 循环覆盖: 逐个删除直到为空
 */
TEST_F(TodoAppTest, DeleteAllTodos) {
    todoApp_->initialize();
    
    todoApp_->addTodo("任务1");
    todoApp_->addTodo("任务2");
    todoApp_->addTodo("任务3");
    
    EXPECT_EQ(todoApp_->getTotalCount(), 3);
    
    // 逐个删除（总是删除第一个）
    while (todoApp_->getTotalCount() > 0) {
        todoApp_->deleteTodo(0);
    }
    
    // 验证
    EXPECT_EQ(todoApp_->getTotalCount(), 0);
}

/**
 * 测试用例 15: 待办项标记后删除
 * 测试目标: 验证已完成的待办项删除后统计正确
 * 
 * 覆盖分析:
 * - 路径覆盖: 标记后删除的路径
 */
TEST_F(TodoAppTest, DeleteCompletedTodo) {
    todoApp_->initialize();
    
    todoApp_->addTodo("任务1");
    todoApp_->addTodo("任务2");
    
    todoApp_->toggleTodo(0);
    EXPECT_EQ(todoApp_->getCompletedCount(), 1);
    
    // 删除已完成的
    todoApp_->deleteTodo(0);
    
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
    EXPECT_EQ(todoApp_->getCompletedCount(), 0);
}

/**
 * 测试用例 16: 按钮点击事件测试
 * 测试目标: 验证点击添加按钮时能正确触发addTodo
 *
 * 覆盖分析:
 * - 事件系统测试: 事件分发和处理
 * - 点击检测: isComponentClicked方法
 * - 事件日志: 事件日志的记录和读取
 */
TEST_F(TodoAppTest, ButtonClickEventTest) {
    // 初始化应用
    todoApp_->initialize();

    // 确保布局完成
    logicLayer_->runFrame();

    // 获取组件句柄
    auto addButtonHandle = todoApp_->getAddButton();
    auto inputBoxHandle = todoApp_->getInputBox();

    // 获取EventDispatcher
    auto& dispatcher = logicLayer_->getEventDispatcher();
    auto& storage = logicLayer_->getStorage();

    // ==== 第一步：获取输入框位置，设置焦点 ====
    auto* inputEntry = storage.getComponent(inputBoxHandle);
    ASSERT_TRUE(inputEntry != nullptr);

    // 计算输入框的绝对位置
    float inputAbsX = inputEntry->layoutResult.x;
    float inputAbsY = inputEntry->layoutResult.y;

    int32_t inputParentIdx = inputEntry->parentIndex;
    while (inputParentIdx != -1) {
        const auto* parentEntry = storage.getComponentByIndex(inputParentIdx);
        if (!parentEntry) break;

        inputAbsX += parentEntry->layoutResult.x;
        inputAbsY += parentEntry->layoutResult.y;

        inputParentIdx = parentEntry->parentIndex;
    }

    // 点击输入框设置焦点
    dispatcher.onMouseDown(inputAbsX + 5, inputAbsY + 5, 0);

    // ==== 第二步：模拟文本输入 ====
    logicLayer_->dispatchTextInput("测试待办");

    // 调用update处理文本输入
    todoApp_->update();

    // ==== 第三步：获取按钮位置并点击 ====
    auto* buttonEntry = storage.getComponent(addButtonHandle);
    ASSERT_TRUE(buttonEntry != nullptr);

    // 计算按钮的绝对位置
    float btnAbsX = buttonEntry->layoutResult.x;
    float btnAbsY = buttonEntry->layoutResult.y;

    int32_t btnParentIdx = buttonEntry->parentIndex;
    while (btnParentIdx != -1) {
        const auto* parentEntry = storage.getComponentByIndex(btnParentIdx);
        if (!parentEntry) break;

        btnAbsX += parentEntry->layoutResult.x;
        btnAbsY += parentEntry->layoutResult.y;

        btnParentIdx = parentEntry->parentIndex;
    }

    // 点击按钮中心位置
    float clickX = btnAbsX + buttonEntry->layoutResult.width / 2.0f;
    float clickY = btnAbsY + buttonEntry->layoutResult.height / 2.0f;

    // 模拟点击
    logicLayer_->dispatchClick(clickX, clickY, 0);

    // 调用update处理点击事件
    todoApp_->update();

    // 验证是否添加了待办项
    EXPECT_EQ(todoApp_->getTotalCount(), 1);
}

} // namespace test
} // namespace jaether
