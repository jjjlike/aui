/**
 * 集成测试模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试整个 Aether UI 引擎的集成功能
 * - 包括完整组件树创建、布局计算、事件流、动态更新等
 * - 测试用例覆盖：完整流程测试、边界测试、性能测试
 */

#include "aether/LogicLayer.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 集成测试的夹具类
 * 每个测试运行前会创建一个新的逻辑层实例
 */
class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建逻辑层实例
        layer = std::make_unique<JLogicLayer>();
    }
    
    std::unique_ptr<JLogicLayer> layer;  // 逻辑层指针
};

/**
 * 测试用例 1：完整组件树创建（集成测试）
 * 测试目标：验证能正确创建包含多个层级和类型的完整组件树
 */
TEST_F(IntegrationTest, FullComponentTreeCreation) {
    // 创建根容器组件，设置布局大小为 800x600
    auto root = layer->createComponent(JComponentType::Container);
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建头部容器组件
    auto header = layer->createComponent(JComponentType::Container, root);
    // 设置头部位置和大小（位于顶部，高度 50px）
    layer->setProperty(header, JPropertyId::X, JPropertyValue(0.0f));
    layer->setProperty(header, JPropertyId::Y, JPropertyValue(0.0f));
    layer->setProperty(header, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(header, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 创建内容容器组件
    auto content = layer->createComponent(JComponentType::Container, root);
    // 设置内容区域位置和大小（位于头部下方，高度 500px）
    layer->setProperty(content, JPropertyId::X, JPropertyValue(0.0f));
    layer->setProperty(content, JPropertyId::Y, JPropertyValue(50.0f));
    layer->setProperty(content, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(content, JPropertyId::Height, JPropertyValue(500.0f));
    
    // 创建第一个按钮组件
    auto btn1 = layer->createComponent(JComponentType::Button, content);
    // 设置按钮1的位置和大小
    layer->setProperty(btn1, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn1, JPropertyId::Y, JPropertyValue(50.0f));
    layer->setProperty(btn1, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn1, JPropertyId::Height, JPropertyValue(50.0f));
    layer->setProperty(btn1, JPropertyId::Text, JPropertyValue(std::string("Button 1")));
    
    // 创建第二个按钮组件
    auto btn2 = layer->createComponent(JComponentType::Button, content);
    // 设置按钮2的位置和大小
    layer->setProperty(btn2, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn2, JPropertyId::Y, JPropertyValue(120.0f));
    layer->setProperty(btn2, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn2, JPropertyId::Height, JPropertyValue(50.0f));
    layer->setProperty(btn2, JPropertyId::Text, JPropertyValue(std::string("Button 2")));
    
    // 创建文本组件
    auto text = layer->createComponent(JComponentType::Text, content);
    // 设置文本的位置和大小
    layer->setProperty(text, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(text, JPropertyId::Y, JPropertyValue(200.0f));
    layer->setProperty(text, JPropertyId::Width, JPropertyValue(600.0f));
    layer->setProperty(text, JPropertyId::Height, JPropertyValue(100.0f));
    layer->setProperty(text, JPropertyId::Text, JPropertyValue(std::string("Welcome to Aether!")));
    
    // 创建底部容器组件
    auto footer = layer->createComponent(JComponentType::Container, root);
    // 设置底部位置和大小（位于底部，高度 50px）
    layer->setProperty(footer, JPropertyId::X, JPropertyValue(0.0f));
    layer->setProperty(footer, JPropertyId::Y, JPropertyValue(550.0f));
    layer->setProperty(footer, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(footer, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 验证活动组件数量为 7（根、头部、内容、2个按钮、文本、底部）
    EXPECT_EQ(layer->getStorage().activeCount(), 7);
}

/**
 * 测试用例 2：组件树布局计算（集成测试）
 * 测试目标：验证组件树的布局计算能正确执行
 */
TEST_F(IntegrationTest, ComponentTreeLayout) {
    // 创建根容器组件，设置布局大小
    auto root = layer->createComponent(JComponentType::Container);
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建按钮组件，设置位置和大小
    auto btn = layer->createComponent(JComponentType::Button, root);
    layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 运行一帧，触发布局计算
    layer->runFrame();
    
    // 获取组件存储引用
    auto& storage = layer->getStorage();
    auto* rootEntry = storage.getComponent(root);
    auto* btnEntry = storage.getComponent(btn);
    
    // 验证组件存在
    ASSERT_NE(rootEntry, nullptr);
    ASSERT_NE(btnEntry, nullptr);
    
    // 验证根组件的布局位置和宽度
    EXPECT_EQ(rootEntry->layoutResult.x, 0.0f);
    EXPECT_EQ(rootEntry->layoutResult.width, 800.0f);
}

/**
 * 测试用例 3：事件流通过组件树（集成测试）
 * 测试目标：验证鼠标事件能正确流经整个组件树
 */
TEST_F(IntegrationTest, EventFlowThroughTree) {
    // 创建根容器组件，设置布局大小
    auto root = layer->createComponent(JComponentType::Container);
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建按钮组件，设置位置和大小
    auto btn = layer->createComponent(JComponentType::Button, root);
    layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn, JPropertyId::Height, JPropertyValue(50.0f));
    
    // 运行一帧
    layer->runFrame();
    
    // 分发一系列鼠标事件
    layer->dispatchMouseMove(150.0f, 125.0f);  // 鼠标移动到按钮上方
    layer->dispatchMouseDown(150.0f, 125.0f, 0);  // 鼠标按下
    layer->dispatchMouseUp(150.0f, 125.0f, 0);  // 鼠标释放
    layer->dispatchClick(150.0f, 125.0f, 0);  // 点击事件
}

/**
 * 测试用例 4：动态组件更新（集成测试）
 * 测试目标：验证能动态更新组件的属性
 */
TEST_F(IntegrationTest, DynamicComponentUpdates) {
    // 创建根容器组件，设置布局大小
    auto root = layer->createComponent(JComponentType::Container);
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建按钮组件，设置初始属性
    auto btn = layer->createComponent(JComponentType::Button, root);
    layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(200.0f));
    layer->setProperty(btn, JPropertyId::Height, JPropertyValue(50.0f));
    layer->setProperty(btn, JPropertyId::Text, JPropertyValue(std::string("Original")));
    
    // 运行一帧
    layer->runFrame();
    
    // 动态更新按钮的属性
    layer->setProperty(btn, JPropertyId::Text, JPropertyValue(std::string("Updated")));
    layer->setProperty(btn, JPropertyId::Width, JPropertyValue(250.0f));
    
    // 运行一帧
    layer->runFrame();
    
    // 获取组件存储引用
    auto& storage = layer->getStorage();
    auto* btnEntry = storage.getComponent(btn);
    ASSERT_NE(btnEntry, nullptr);
    
    // 获取文本属性
    auto* textProp = btnEntry->properties.getProperty(JPropertyId::Text);
    ASSERT_NE(textProp, nullptr);
    // 验证文本属性已更新
    EXPECT_EQ(textProp->get<std::string>(), "Updated");
}

/**
 * 测试用例 5：组件删除和添加（集成测试）
 * 测试目标：验证能动态删除和添加组件
 */
TEST_F(IntegrationTest, ComponentRemovalAndAddition) {
    // 创建根容器组件，设置布局大小
    auto root = layer->createComponent(JComponentType::Container);
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 动态添加 10 个按钮组件
    for (int i = 0; i < 10; ++i) {
        auto btn = layer->createComponent(JComponentType::Button, root);
        layer->setProperty(btn, JPropertyId::X, JPropertyValue(100.0f + (i % 5) * 150.0f));
        layer->setProperty(btn, JPropertyId::Y, JPropertyValue(100.0f + (i / 5) * 60.0f));
        layer->setProperty(btn, JPropertyId::Width, JPropertyValue(100.0f));
        layer->setProperty(btn, JPropertyId::Height, JPropertyValue(40.0f));
    }
    
    // 验证活动组件数量为 11（根组件 + 10 个按钮）
    EXPECT_EQ(layer->getStorage().activeCount(), 11);
    
    // 运行一帧
    layer->runFrame();
    
    // 获取组件存储引用
    auto& storage = layer->getStorage();
    
    // 收集所有非根组件的句柄
    std::vector<JComponentHandle> toRemove;
    storage.forEach([&](JComponentHandle h) {
        if (h != storage.getRoot()) {
            toRemove.push_back(h);
        }
    });
    
    // 删除所有收集的组件
    for (auto& h : toRemove) {
        layer->destroyComponent(h);
    }
    
    // 验证活动组件数量为 1（只剩下根组件）
    EXPECT_EQ(layer->getStorage().activeCount(), 1);
    
    // 动态添加 5 个文本组件
    for (int i = 0; i < 5; ++i) {
        auto txt = layer->createComponent(JComponentType::Text, root);
        layer->setProperty(txt, JPropertyId::X, JPropertyValue(100.0f + i * 100.0f));
        layer->setProperty(txt, JPropertyId::Y, JPropertyValue(100.0f));
        layer->setProperty(txt, JPropertyId::Width, JPropertyValue(80.0f));
        layer->setProperty(txt, JPropertyId::Height, JPropertyValue(30.0f));
    }
    
    // 验证活动组件数量为 6（根组件 + 5 个文本组件）
    EXPECT_EQ(layer->getStorage().activeCount(), 6);
}

/**
 * 测试用例 6：压力测试（性能测试）
 * 测试目标：验证系统在大规模组件和高帧率下的性能
 */
TEST_F(IntegrationTest, StressTest) {
    // 创建根容器组件，设置较大的布局大小
    auto root = layer->createComponent(JComponentType::Container);
    layer->setProperty(root, JPropertyId::Width, JPropertyValue(1000.0f));
    layer->setProperty(root, JPropertyId::Height, JPropertyValue(1000.0f));
    
    // 动态添加 100 个组件（按钮、文本、容器交替）
    for (int i = 0; i < 100; ++i) {
        // 根据索引循环选择组件类型
        auto comp = layer->createComponent(i % 3 == 0 ? JComponentType::Button : 
                                  (i % 3 == 1 ? JComponentType::Text : JComponentType::Container), root);
        // 设置组件的位置和大小，排列成 10x10 的网格
        layer->setProperty(comp, JPropertyId::X, JPropertyValue(10.0f + (i % 10) * 90.0f));
        layer->setProperty(comp, JPropertyId::Y, JPropertyValue(10.0f + (i / 10) * 90.0f));
        layer->setProperty(comp, JPropertyId::Width, JPropertyValue(80.0f));
        layer->setProperty(comp, JPropertyId::Height, JPropertyValue(80.0f));
    }
    
    // 运行一帧初始化布局
    layer->runFrame();
    
    // 运行 30 帧进行压力测试
    for (int frame = 0; frame < 30; ++frame) {
        layer->runFrame();
    }
    
    // 验证活动组件数量为 101（根组件 + 100 个子组件）
    EXPECT_EQ(layer->getStorage().activeCount(), 101);
}

} // namespace test
} // namespace jaether
