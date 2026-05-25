/**
 * 逻辑层模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JLogicLayer 类的逻辑层功能
 * - 包括组件管理、属性设置、帧循环、事件分发等
 * - 测试用例覆盖：正常逻辑、边界情况、集成测试
 */

#include "aether/LogicLayer.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 逻辑层测试的夹具类
 * 每个测试运行前会创建一个新的逻辑层实例
 */
class LogicLayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建逻辑层实例
        layer = std::make_unique<JLogicLayer>();
    }
    
    std::unique_ptr<JLogicLayer> layer;  // 逻辑层指针
};

/**
 * 测试用例 1：初始化逻辑层（正常逻辑测试）
 * 测试目标：验证能正确初始化逻辑层
 */
TEST_F(LogicLayerTest, InitializeLayer) {
    // 验证逻辑层指针不为空
    EXPECT_TRUE(layer != nullptr);
}

/**
 * 测试用例 2：创建根组件（正常逻辑测试）
 * 测试目标：验证能正确创建根组件
 */
TEST_F(LogicLayerTest, CreateRootComponent) {
    // 创建一个容器组件作为根组件
    auto root = layer->createComponent(JComponentType::Container);
    
    // 验证返回的组件句柄有效
    EXPECT_TRUE(root.isValid());
    // 验证根组件与存储中的根组件相同
    EXPECT_EQ(layer->getStorage().getRoot(), root);
}

/**
 * 测试用例 3：创建多个组件（正常逻辑测试）
 * 测试目标：验证能正确创建多个组件
 */
TEST_F(LogicLayerTest, CreateMultipleComponents) {
    // 创建根容器组件
    auto root = layer->createComponent(JComponentType::Container);
    // 创建按钮组件，作为根组件的子组件
    auto btn = layer->createComponent(JComponentType::Button, root);
    // 创建文本组件，作为根组件的子组件
    auto txt = layer->createComponent(JComponentType::Text, root);
    
    // 验证所有组件句柄有效
    EXPECT_TRUE(root.isValid());
    EXPECT_TRUE(btn.isValid());
    EXPECT_TRUE(txt.isValid());
    
    // 验证活动组件数量为 3（根、按钮、文本）
    EXPECT_EQ(layer->getStorage().activeCount(), 3);
}

/**
 * 测试用例 4：设置组件属性（正常逻辑测试）
 * 测试目标：验证能正确设置组件的各种属性
 */
TEST_F(LogicLayerTest, SetProperty) {
    // 创建一个容器组件
    auto comp = layer->createComponent(JComponentType::Container);
    
    // 设置宽度属性为 200.0f
    layer->setProperty(comp, JPropertyId::Width, JPropertyValue(200.0f));
    // 设置高度属性为 100.0f
    layer->setProperty(comp, JPropertyId::Height, JPropertyValue(100.0f));
    // 设置文本属性为 "Test"
    layer->setProperty(comp, JPropertyId::Text, JPropertyValue(std::string("Test")));
    
    // 获取组件存储引用
    auto& storage = layer->getStorage();
    auto* entry = storage.getComponent(comp);
    // 验证组件存在
    ASSERT_NE(entry, nullptr);
    
    // 获取各个属性
    auto* widthProp = entry->properties.getProperty(JPropertyId::Width);
    auto* heightProp = entry->properties.getProperty(JPropertyId::Height);
    auto* textProp = entry->properties.getProperty(JPropertyId::Text);
    
    // 验证所有属性都存在
    ASSERT_NE(widthProp, nullptr);
    ASSERT_NE(heightProp, nullptr);
    ASSERT_NE(textProp, nullptr);
    
    // 验证属性值正确
    EXPECT_EQ(widthProp->get<float>(), 200.0f);
    EXPECT_EQ(heightProp->get<float>(), 100.0f);
    EXPECT_EQ(textProp->get<std::string>(), "Test");
}

/**
 * 测试用例 5：运行帧循环（正常逻辑测试）
 * 测试目标：验证能正确运行帧循环并更新布局
 */
TEST_F(LogicLayerTest, RunFrame) {
    // 创建根容器组件
    auto root = layer->createComponent(JComponentType::Container);
    // 设置根组件的布局大小
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建一个按钮组件
    auto btn = layer->createComponent(JComponentType::Button, root);
    // 设置按钮的位置和大小属性
    layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 运行一帧
    layer->runFrame();
    
    // 获取组件存储引用
    auto& storage = layer->getStorage();
    auto* rootEntry = storage.getComponent(root);
    auto* btnEntry = storage.getComponent(btn);
    
    // 验证组件存在
    ASSERT_NE(rootEntry, nullptr);
    ASSERT_NE(btnEntry, nullptr);
    
    // 验证根组件的布局宽度大于 0
    EXPECT_GT(rootEntry->layoutResult.width, 0);
}

/**
 * 测试用例 6：销毁组件（正常逻辑测试）
 * 测试目标：验证能正确销毁组件
 */
TEST_F(LogicLayerTest, DestroyComponent) {
    // 创建根容器组件
    auto root = layer->createComponent(JComponentType::Container);
    // 创建按钮组件
    auto child = layer->createComponent(JComponentType::Button, root);
    
    // 验证按钮组件句柄有效
    EXPECT_TRUE(layer->getStorage().isValid(child));
    
    // 销毁按钮组件
    layer->destroyComponent(child);
    
    // 验证按钮组件句柄无效
    EXPECT_FALSE(layer->getStorage().isValid(child));
}

/**
 * 测试用例 7：多帧运行（边界测试）
 * 测试目标：验证能正确运行多帧
 */
TEST_F(LogicLayerTest, MultipleFrames) {
    // 创建根容器组件
    auto root = layer->createComponent(JComponentType::Container);
    // 设置根组件的布局大小
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 运行 10 帧
    for (int i = 0; i < 10; ++i) {
        layer->runFrame();
    }
    
    // 获取组件存储引用
    auto& storage = layer->getStorage();
    auto* rootEntry = storage.getComponent(root);
    
    // 验证根组件存在
    ASSERT_NE(rootEntry, nullptr);
    // 验证布局宽度保持不变
    EXPECT_EQ(rootEntry->layoutResult.width, 800.0f);
}

/**
 * 测试用例 8：分发鼠标移动事件（正常逻辑测试）
 * 测试目标：验证能正确分发鼠标移动事件
 */
TEST_F(LogicLayerTest, DispatchMouseMove) {
    // 创建根容器组件
    auto root = layer->createComponent(JComponentType::Container);
    // 设置根组件的布局大小
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建一个按钮组件
    auto btn = layer->createComponent(JComponentType::Button, root);
    // 设置按钮的位置和大小属性
    layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 运行一帧
    layer->runFrame();
    
    // 分发鼠标移动事件
    layer->dispatchMouseMove(150.0f, 125.0f);
}

/**
 * 测试用例 9：分发点击事件（正常逻辑测试）
 * 测试目标：验证能正确分发点击事件
 */
TEST_F(LogicLayerTest, DispatchClick) {
    // 创建根容器组件
    auto root = layer->createComponent(JComponentType::Container);
    // 设置根组件的布局大小
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建一个按钮组件
    auto btn = layer->createComponent(JComponentType::Button, root);
    // 设置按钮的位置和大小属性
    layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 运行一帧
    layer->runFrame();
    
    // 分发点击事件
    layer->dispatchClick(150.0f, 125.0f, 0);
}

/**
 * 测试用例 10：分发键盘按键事件（正常逻辑测试）
 * 测试目标：验证能正确分发键盘按键事件
 */
TEST_F(LogicLayerTest, DispatchKeyDown) {
    // 分发回车键按下事件
    layer->dispatchKeyDown(13);
}

/**
 * 测试用例 11：分发文本输入事件（正常逻辑测试）
 * 测试目标：验证能正确分发文本输入事件
 */
TEST_F(LogicLayerTest, DispatchTextInput) {
    // 分发文本输入事件
    layer->dispatchTextInput("Hello");
}

} // namespace test
} // namespace jaether
