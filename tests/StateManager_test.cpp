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
 * 状态管理器模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JStateManager 类的状态管理功能
 * - 包括组件创建销毁、属性设置、批量更新、观察者模式等
 * - 测试用例覆盖：正常逻辑、边界情况、通知机制
 */

#include "aether/StateManager.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 状态管理器测试的夹具类
 * 每个测试运行前会创建新的组件存储和状态管理器实例
 */
class StateManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储实例
        storage = std::make_unique<JComponentStorage>();
        // 创建状态管理器实例，关联到组件存储
        stateManager = std::make_unique<JStateManager>(*storage);
    }
    
    std::unique_ptr<JComponentStorage> storage;      // 组件存储指针
    std::unique_ptr<JStateManager> stateManager;   // 状态管理器指针
};

/**
 * 测试用例 1：创建组件（正常逻辑测试）
 * 测试目标：验证能正确创建组件
 */
TEST_F(StateManagerTest, CreateComponent) {
    // 创建一个按钮组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    // 验证返回的组件句柄有效
    EXPECT_TRUE(handle.isValid());
    // 验证活动组件数量为 1
    EXPECT_EQ(storage->activeCount(), 1);
}

/**
 * 测试用例 2：创建带父组件的组件（正常逻辑测试）
 * 测试目标：验证能正确创建具有父子关系的组件
 */
TEST_F(StateManagerTest, CreateComponentWithParent) {
    // 创建一个容器组件作为父组件
    auto parent = stateManager->createComponent(JComponentType::Container);
    // 创建一个按钮组件作为子组件
    auto child = stateManager->createComponent(JComponentType::Button, parent);
    
    // 验证子组件句柄有效
    EXPECT_TRUE(child.isValid());
    // 验证活动组件数量为 2（父组件和子组件）
    EXPECT_EQ(storage->activeCount(), 2);
}

/**
 * 测试用例 3：销毁组件（正常逻辑测试）
 * 测试目标：验证能正确销毁组件
 */
TEST_F(StateManagerTest, DestroyComponent) {
    // 创建一个按钮组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    // 验证返回的组件句柄有效
    EXPECT_TRUE(storage->isValid(handle));
    
    // 销毁组件
    stateManager->destroyComponent(handle);
    
    // 验证组件句柄无效
    EXPECT_FALSE(storage->isValid(handle));
    // 验证活动组件数量为 0
    EXPECT_EQ(storage->activeCount(), 0);
}

/**
 * 测试用例 4：设置属性（正常逻辑测试）
 * 测试目标：验证能正确设置组件属性
 */
TEST_F(StateManagerTest, SetProperty) {
    // 创建一个按钮组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    
    // 设置宽度属性为 100
    stateManager->setProperty(handle, JPropertyId::Width, JPropertyValue(100));
    
    // 获取宽度属性
    auto* value = stateManager->getProperty(handle, JPropertyId::Width);
    // 验证属性存在
    ASSERT_NE(value, nullptr);
    // 验证属性值为 100
    EXPECT_EQ(value->get<int>(), 100);
}

/**
 * 测试用例 5：设置可见属性（正常逻辑测试）
 * 测试目标：验证能正确设置组件的可见属性
 */
TEST_F(StateManagerTest, SetVisibleProperty) {
    // 创建一个按钮组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    // 获取组件条目
    auto* entry = storage->getComponent(handle);
    
    // 验证初始可见性为 true
    EXPECT_TRUE(entry->visible);
    
    // 设置可见属性为 false
    stateManager->setProperty(handle, JPropertyId::Visible, JPropertyValue(false));
    
    // 验证可见性已改变
    EXPECT_FALSE(entry->visible);
}

/**
 * 测试用例 6：设置启用属性（正常逻辑测试）
 * 测试目标：验证能正确设置组件的启用属性
 */
TEST_F(StateManagerTest, SetEnabledProperty) {
    // 创建一个按钮组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    // 获取组件条目
    auto* entry = storage->getComponent(handle);
    
    // 验证初始启用状态为 true
    EXPECT_TRUE(entry->enabled);
    
    // 设置启用属性为 false
    stateManager->setProperty(handle, JPropertyId::Enabled, JPropertyValue(false));
    
    // 验证启用状态已改变
    EXPECT_FALSE(entry->enabled);
}

/**
 * 测试用例 7：批量更新（分支测试）
 * 测试目标：验证批量更新机制能正确合并多次属性变更
 */
TEST_F(StateManagerTest, BatchUpdates) {
    // 创建两个组件
    auto handle1 = stateManager->createComponent(JComponentType::Button);
    auto handle2 = stateManager->createComponent(JComponentType::Text);
    
    // 开始批量更新
    stateManager->beginBatch();
    // 验证在批量更新中
    EXPECT_TRUE(stateManager->isInBatch());
    
    // 设置多个属性
    stateManager->setProperty(handle1, JPropertyId::Width, JPropertyValue(100));
    stateManager->setProperty(handle2, JPropertyId::Height, JPropertyValue(50));
    
    // 结束批量更新
    stateManager->endBatch();
    // 验证不在批量更新中
    EXPECT_FALSE(stateManager->isInBatch());
}

/**
 * 测试用例 8：获取不存在的属性（边界测试）
 * 测试目标：验证获取不存在的属性时返回空指针
 */
TEST_F(StateManagerTest, GetPropertyNonExistent) {
    // 创建一个按钮组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    
    // 尝试获取不存在的文本属性
    auto* value = stateManager->getProperty(handle, JPropertyId::Text);
    // 验证返回空指针
    EXPECT_EQ(value, nullptr);
}

/**
 * 测试用例 9：多个组件（正常逻辑测试）
 * 测试目标：验证能正确管理多个组件
 */
TEST_F(StateManagerTest, MultipleComponents) {
    std::vector<JComponentHandle> handles;
    
    // 创建 10 个按钮组件
    for (int i = 0; i < 10; ++i) {
        handles.push_back(stateManager->createComponent(JComponentType::Button));
    }
    
    // 验证活动组件数量为 10
    EXPECT_EQ(storage->activeCount(), 10);
    
    // 为每个组件设置不同的宽度
    for (size_t i = 0; i < handles.size(); ++i) {
        stateManager->setProperty(handles[i], JPropertyId::Width, JPropertyValue(static_cast<int>(i)));
    }
}

/**
 * 测试观察者类
 * 用于测试观察者模式的通知机制
 */
class TestObserver : public JStateObserver {
public:
    int propertyChangedCount = 0;      // 属性变更计数
    int componentCreatedCount = 0;     // 组件创建计数
    int componentDestroyedCount = 0;   // 组件销毁计数
    int layoutCompleteCount = 0;       // 布局完成计数
    
    // 属性变更回调
    void onPropertyChanged(JComponentHandle, JPropertyId, const JPropertyValue&) override {
        propertyChangedCount++;
    }
    
    // 组件创建回调
    void onComponentCreated(JComponentHandle) override {
        componentCreatedCount++;
    }
    
    // 组件销毁回调
    void onComponentDestroyed(JComponentHandle) override {
        componentDestroyedCount++;
    }
    
    // 布局完成回调
    void onLayoutComplete() override {
        layoutCompleteCount++;
    }
};

/**
 * 测试用例 10：观察者通知（正常逻辑测试）
 * 测试目标：验证观察者能正确接收状态变更通知
 */
TEST_F(StateManagerTest, ObserverNotifications) {
    // 创建测试观察者
    TestObserver observer;
    // 注册观察者
    stateManager->addObserver(&observer);
    
    // 创建组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    // 验证观察者收到了创建通知
    EXPECT_EQ(observer.componentCreatedCount, 1);
    
    // 设置属性
    stateManager->setProperty(handle, JPropertyId::Width, JPropertyValue(100));
    // 验证观察者收到了属性变更通知
    EXPECT_GE(observer.propertyChangedCount, 1);
    
    // 销毁组件
    stateManager->destroyComponent(handle);
    // 验证观察者收到了销毁通知
    EXPECT_EQ(observer.componentDestroyedCount, 1);
    
    // 移除观察者
    stateManager->removeObserver(&observer);
}

/**
 * 测试用例 11：多个观察者（分支测试）
 * 测试目标：验证多个观察者能同时接收通知
 */
TEST_F(StateManagerTest, MultipleObservers) {
    // 创建两个测试观察者
    TestObserver observer1;
    TestObserver observer2;
    
    // 注册观察者
    stateManager->addObserver(&observer1);
    stateManager->addObserver(&observer2);
    
    // 创建组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    
    // 验证两个观察者都收到了创建通知
    EXPECT_EQ(observer1.componentCreatedCount, 1);
    EXPECT_EQ(observer2.componentCreatedCount, 1);
}

/**
 * 测试用例 12：移除观察者（正常逻辑测试）
 * 测试目标：验证移除观察者后不再接收通知
 */
TEST_F(StateManagerTest, RemoveObserver) {
    // 创建测试观察者
    TestObserver observer;
    
    // 注册观察者
    stateManager->addObserver(&observer);
    // 立即移除观察者
    stateManager->removeObserver(&observer);
    
    // 创建组件
    auto handle = stateManager->createComponent(JComponentType::Button);
    
    // 验证观察者的计数器仍为 0（没有收到通知）
    EXPECT_EQ(observer.componentCreatedCount, 0);
}

} // namespace test
} // namespace jaether
