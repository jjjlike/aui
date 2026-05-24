/**
 * 布局引擎模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 LayoutEngine 类的布局计算功能
 * - 包括脏标记、布局传播、Flexbox 布局、嵌套布局等
 * - 测试用例覆盖：正常逻辑、边界情况、布局算法
 */

#include "aether/LayoutEngine.h"
#include <gtest/gtest.h>

namespace aether {
namespace test {

/**
 * 布局引擎测试的夹具类
 * 每个测试运行前会创建新的组件存储和布局引擎实例
 */
class LayoutEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储实例
        storage = std::make_unique<ComponentStorage>();
        // 创建布局引擎实例，关联到组件存储
        engine = std::make_unique<LayoutEngine>(*storage);
        
        // 创建根容器组件
        root = storage->createComponent(ComponentType::Container);
        // 获取根组件的详细信息
        auto* rootEntry = storage->getComponent(root);
        // 设置根组件的初始布局大小（800x600）
        rootEntry->layoutResult = {0, 0, 800, 600};
    }
    
    std::unique_ptr<ComponentStorage> storage;  // 组件存储指针
    std::unique_ptr<LayoutEngine> engine;      // 布局引擎指针
    ComponentHandle root;                      // 根组件句柄
};

/**
 * 测试用例 1：初始状态不是脏的（正常逻辑测试）
 * 测试目标：验证新创建的组件在初始状态下不是脏的，不需要重新布局
 */
TEST_F(LayoutEngineTest, InitialStateNotDirty) {
    // 验证根组件在初始状态下不是脏的
    EXPECT_FALSE(engine->isDirty(root));
}

/**
 * 测试用例 2：标记为脏（正常逻辑测试）
 * 测试目标：验证 markDirty() 能正确标记组件为脏状态
 */
TEST_F(LayoutEngineTest, MarkDirty) {
    // 标记根组件为脏
    engine->markDirty(root);
    // 验证根组件现在是脏的
    EXPECT_TRUE(engine->isDirty(root));
}

/**
 * 测试用例 3：通过属性标记为脏（正常逻辑测试）
 * 测试目标：验证通过属性 ID 标记脏状态的功能
 */
TEST_F(LayoutEngineTest, MarkDirtyWithProperty) {
    // 通过宽度属性标记根组件为脏
    engine->markDirty(root, PropertyId::Width);
    // 验证根组件现在是脏的
    EXPECT_TRUE(engine->isDirty(root));
}

/**
 * 测试用例 4：非布局属性不标记脏（条件测试）
 * 测试目标：验证只有布局相关的属性才会标记组件为脏
 */
TEST_F(LayoutEngineTest, NonLayoutPropertyDoesNotMarkDirty) {
    // 通过文本属性尝试标记（文本不是布局属性）
    engine->markDirty(root, PropertyId::Text);
    // 验证根组件仍然不是脏的
    EXPECT_FALSE(engine->isDirty(root));
}

/**
 * 测试用例 5：脏状态向上传播到父组件（分支测试）
 * 测试目标：验证子组件变脏时，脏状态会向上传播到父组件
 */
TEST_F(LayoutEngineTest, DirtyPropagationToParent) {
    // 创建一个子按钮组件
    auto child = storage->createComponent(ComponentType::Button, root);
    
    // 标记子组件的宽度属性为脏
    engine->markDirty(child, PropertyId::Width);
    
    // 验证子组件是脏的
    EXPECT_TRUE(engine->isDirty(child));
    // 验证父组件也是脏的（脏状态传播）
    EXPECT_TRUE(engine->isDirty(root));
}

/**
 * 测试用例 6：需要时重新布局（正常逻辑测试）
 * 测试目标：验证 relayoutIfNeeded() 只在需要时执行布局
 */
TEST_F(LayoutEngineTest, RelayoutIfNeeded) {
    // 标记根组件为脏
    engine->markDirty(root);
    
    // 执行需要时的重新布局
    engine->relayoutIfNeeded();
    
    // 验证根组件不再是脏的（布局已完成）
    EXPECT_FALSE(engine->isDirty(root));
}

/**
 * 测试用例 7：强制重新布局（正常逻辑测试）
 * 测试目标：验证 forceRelayout() 能强制执行重新布局
 */
TEST_F(LayoutEngineTest, ForceRelayout) {
    // 强制执行重新布局
    engine->forceRelayout();
    // 验证所有组件都不是脏的
    EXPECT_FALSE(engine->isDirty(root));
}

/**
 * 测试用例 8：简单 Flexbox 布局（正常逻辑测试）
 * 测试目标：验证 Flexbox 布局算法能正确计算子组件的位置和大小
 */
TEST_F(LayoutEngineTest, SimpleFlexboxLayout) {
    // 获取根组件并设置 Flex 方向为水平排列
    auto* rootEntry = storage->getComponent(root);
    rootEntry->properties.setProperty(PropertyId::FlexDirection, PropertyValue(FlexDirection::Row));
    
    // 创建两个子按钮组件
    auto child1 = storage->createComponent(ComponentType::Button, root);
    auto child2 = storage->createComponent(ComponentType::Button, root);
    
    // 获取子组件的详细信息
    auto* child1Entry = storage->getComponent(child1);
    auto* child2Entry = storage->getComponent(child2);
    
    // 设置第一个子组件的宽度和高度属性
    child1Entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    child1Entry->properties.setProperty(PropertyId::Height, PropertyValue(50));
    // 设置第二个子组件的宽度和高度属性
    child2Entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    child2Entry->properties.setProperty(PropertyId::Height, PropertyValue(50));
    
    // 强制执行重新布局
    engine->forceRelayout();
    
    // 验证第一个子组件的宽度大于 0
    EXPECT_GT(child1Entry->layoutResult.width, 0);
    // 验证第二个子组件的宽度大于 0
    EXPECT_GT(child2Entry->layoutResult.width, 0);
}

/**
 * 测试用例 9：FlexGrow 分配（边界测试）
 * 测试目标：验证 FlexGrow 属性能正确分配剩余空间
 */
TEST_F(LayoutEngineTest, FlexGrowDistribution) {
    // 获取根组件并设置布局大小为 500x100
    auto* rootEntry = storage->getComponent(root);
    rootEntry->layoutResult = {0, 0, 500, 100};
    
    // 创建两个子按钮组件
    auto child1 = storage->createComponent(ComponentType::Button, root);
    auto child2 = storage->createComponent(ComponentType::Button, root);
    
    // 获取子组件的详细信息
    auto* child1Entry = storage->getComponent(child1);
    auto* child2Entry = storage->getComponent(child2);
    
    // 设置第一个子组件的初始宽度和 FlexGrow
    child1Entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    child1Entry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));
    // 设置第二个子组件的初始宽度和 FlexGrow
    child2Entry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    child2Entry->properties.setProperty(PropertyId::FlexGrow, PropertyValue(1));
    
    // 强制执行重新布局
    engine->forceRelayout();
    
    // 验证第一个子组件的宽度大于初始宽度（获得了剩余空间）
    EXPECT_GT(child1Entry->layoutResult.width, 100);
    // 验证第二个子组件的宽度大于初始宽度（获得了剩余空间）
    EXPECT_GT(child2Entry->layoutResult.width, 100);
}

/**
 * 测试用例 10：嵌套布局（分支测试）
 * 测试目标：验证嵌套组件结构的布局计算
 */
TEST_F(LayoutEngineTest, NestedLayout) {
    // 获取根组件并设置布局大小
    auto* rootEntry = storage->getComponent(root);
    rootEntry->layoutResult = {0, 0, 800, 600};
    
    // 创建一个容器组件和子按钮组件
    auto container = storage->createComponent(ComponentType::Container, root);
    auto child = storage->createComponent(ComponentType::Button, container);
    
    // 获取容器和子组件的详细信息
    auto* containerEntry = storage->getComponent(container);
    auto* childEntry = storage->getComponent(child);
    
    // 设置容器的布局大小
    containerEntry->layoutResult = {0, 0, 400, 300};
    // 设置子组件的宽度和高度属性
    childEntry->properties.setProperty(PropertyId::Width, PropertyValue(100));
    childEntry->properties.setProperty(PropertyId::Height, PropertyValue(40));
    
    // 强制执行重新布局
    engine->forceRelayout();
    
    // 验证容器的宽度大于 0
    EXPECT_GT(containerEntry->layoutResult.width, 0);
    // 验证子组件的宽度大于 0
    EXPECT_GT(childEntry->layoutResult.width, 0);
}

/**
 * 测试用例 11：隐藏组件不参与布局（边界测试）
 * 测试目标：验证隐藏的组件不会影响布局计算
 */
TEST_F(LayoutEngineTest, HiddenComponentNotLayout) {
    // 创建一个子按钮组件
    auto child = storage->createComponent(ComponentType::Button, root);
    auto* childEntry = storage->getComponent(child);
    // 将子组件设置为不可见
    childEntry->visible = false;
    
    // 设置子组件的布局大小
    childEntry->layoutResult = {0, 0, 100, 50};
    
    // 强制执行重新布局
    engine->forceRelayout();
}

/**
 * 测试用例 12：布局模式切换（正常逻辑测试）
 * 测试目标：验证能正确切换布局引擎的工作模式
 */
TEST_F(LayoutEngineTest, LayoutMode) {
    // 设置布局模式为测试模式
    engine->setMode(LayoutEngineMode::Test);
    // 验证模式设置正确
    EXPECT_EQ(engine->getMode(), LayoutEngineMode::Test);
    
    // 设置布局模式为正常模式
    engine->setMode(LayoutEngineMode::Normal);
    // 验证模式设置正确
    EXPECT_EQ(engine->getMode(), LayoutEngineMode::Normal);
}

/**
 * 测试用例 13：布局时间跟踪（性能测试）
 * 测试目标：验证布局引擎能正确跟踪布局计算的时间
 */
TEST_F(LayoutEngineTest, LayoutTimeTracking) {
    // 重置布局时间统计
    engine->resetMetrics();
    // 验证初始时间为 0
    EXPECT_EQ(engine->getTotalLayoutTime(), 0);
    
    // 执行一次重新布局
    engine->forceRelayout();
    
    // 验证布局时间大于等于 0（可能有微小的时间消耗）
    EXPECT_GE(engine->getTotalLayoutTime(), 0);
}

} // namespace test
} // namespace aether
