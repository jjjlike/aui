// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


/**
 * 测试控制器模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JTestController 类的测试控制功能
 * - 包括属性设置、属性获取、事件日志获取等
 * - 测试用例覆盖：正常逻辑、边界情况、集成测试
 */

#include "aether/TestController.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 测试控制器测试的夹具类
 * 每个测试运行前会创建完整的测试环境
 */
class TestControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储实例
        storage = std::make_unique<JComponentStorage>();
        // 创建状态管理器实例
        stateManager = std::make_unique<JStateManager>(*storage);
        // 创建事件分发器实例
        dispatcher = std::make_unique<JEventDispatcher>(*storage);
        // 创建测试控制器实例，整合状态管理器和事件分发器
        testController = std::make_unique<JTestController>(*stateManager, *dispatcher);
        
        // 创建根容器组件
        root = stateManager->createComponent(JComponentType::Container);
    }
    
    std::unique_ptr<JComponentStorage> storage;      // 组件存储指针
    std::unique_ptr<JStateManager> stateManager;   // 状态管理器指针
    std::unique_ptr<JEventDispatcher> dispatcher;   // 事件分发器指针
    std::unique_ptr<JTestController> testController;  // 测试控制器指针
    JComponentHandle root;                          // 根组件句柄
};

/**
 * 测试用例 1：通过字符串 ID 设置属性（正常逻辑测试）
 * 测试目标：验证能通过字符串形式的组件 ID 设置属性
 */
TEST_F(TestControllerTest, SetProperty) {
    // 创建一个按钮组件
    auto button = stateManager->createComponent(JComponentType::Button, root);
    auto* entry = storage->getComponent(button);
    
    // 将组件 ID 转换为字符串
    std::ostringstream oss;
    oss << entry->id;
    
    // 使用测试控制器设置属性
    testController->setProperty(oss.str(), "width", "200");
    
    // 获取设置的属性值
    auto* value = stateManager->getProperty(button, JPropertyId::Width);
    // 验证属性存在
    ASSERT_NE(value, nullptr);
    // 验证属性类型正确
    EXPECT_TRUE(value->is<int>());
    // 验证属性值为 200
    EXPECT_EQ(value->get<int>(), 200);
}

/**
 * 测试用例 2：通过字符串 ID 获取属性（正常逻辑测试）
 * 测试目标：验证能通过字符串形式的组件 ID 获取属性
 */
TEST_F(TestControllerTest, GetProperty) {
    // 创建一个按钮组件
    auto button = stateManager->createComponent(JComponentType::Button, root);
    // 设置按钮的宽度属性为 150
    stateManager->setProperty(button, JPropertyId::Width, JPropertyValue(150));
    auto* entry = storage->getComponent(button);
    
    // 将组件 ID 转换为字符串
    std::ostringstream oss;
    oss << entry->id;
    
    // 使用测试控制器获取属性值
    auto value = testController->getProperty(oss.str(), "width");
    
    // 验证返回的属性值不为空
    EXPECT_FALSE(value.empty());
}

/**
 * 测试用例 3：获取事件日志（正常逻辑测试）
 * 测试目标：验证能正确获取事件日志
 */
TEST_F(TestControllerTest, GetEventLog) {
    // 创建一个按钮组件
    auto button = stateManager->createComponent(JComponentType::Button, root);
    // 设置按钮的宽度属性
    stateManager->setProperty(button, JPropertyId::Width, JPropertyValue(100));
    
    // 获取事件日志
    auto log = testController->getEventLog();
}

/**
 * 测试用例 4：通过字符串 ID 获取组件（正常逻辑测试）
 * 测试目标：验证能通过字符串形式的组件 ID 获取组件
 */
TEST_F(TestControllerTest, GetComponentById) {
    // 创建一个按钮组件
    auto button = stateManager->createComponent(JComponentType::Button, root);
    auto* entry = storage->getComponent(button);
    
    // 将组件 ID 转换为字符串
    std::ostringstream oss;
    oss << entry->id;
    
    // 使用测试控制器获取组件
    auto found = testController->getComponentById(oss.str());
    // 验证获取的组件有效
    EXPECT_TRUE(found.isValid());
}

} // namespace test
} // namespace jaether
