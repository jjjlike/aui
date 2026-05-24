/**
 * 快照序列化模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 SnapshotSerializer 类的序列化功能
 * - 包括快照捕获、JSON 序列化、二进制序列化、快照比较等
 * - 测试用例覆盖：正常逻辑、边界情况、往返序列化
 */

#include "aether/Snapshot.h"
#include <gtest/gtest.h>

namespace aether {
namespace test {

/**
 * 快照测试的夹具类
 * 每个测试运行前会创建新的组件存储实例
 */
class SnapshotTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储实例
        storage = std::make_unique<ComponentStorage>();
        
        // 创建根容器组件
        root = storage->createComponent(ComponentType::Container);
        // 获取根组件的详细信息
        auto* rootEntry = storage->getComponent(root);
        // 设置根组件的布局大小（800x600）
        rootEntry->layoutResult = {0, 0, 800, 600};
    }
    
    std::unique_ptr<ComponentStorage> storage;  // 组件存储指针
    ComponentHandle root;                      // 根组件句柄
};

/**
 * 测试用例 1：捕获空存储（正常逻辑测试）
 * 测试目标：验证能正确捕获空存储的快照
 */
TEST_F(SnapshotTest, CaptureEmptyStorage) {
    // 捕获组件存储的快照
    auto snapshot = SnapshotSerializer::capture(*storage);
    
    // 验证快照中的组件计数为 1（只有根组件）
    EXPECT_EQ(snapshot.componentCount, 1);
    // 验证快照中的组件向量大小为 1
    EXPECT_EQ(snapshot.components.size(), 1);
}

/**
 * 测试用例 2：捕获带组件的存储（正常逻辑测试）
 * 测试目标：验证能正确捕获包含组件的存储快照
 */
TEST_F(SnapshotTest, CaptureWithComponents) {
    // 创建一个按钮组件作为根组件的子组件
    auto button = storage->createComponent(ComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为可见
    buttonEntry->visible = true;
    // 设置按钮为启用状态
    buttonEntry->enabled = true;
    
    // 捕获组件存储的快照
    auto snapshot = SnapshotSerializer::capture(*storage);
    
    // 验证快照中的组件计数为 2（根组件和按钮）
    EXPECT_EQ(snapshot.componentCount, 2);
    // 验证快照中的组件向量大小为 2
    EXPECT_EQ(snapshot.components.size(), 2);
}

/**
 * 测试用例 3：序列化为 JSON（正常逻辑测试）
 * 测试目标：验证能正确将快照序列化为 JSON 格式
 */
TEST_F(SnapshotTest, ToJSON) {
    // 捕获组件存储的快照
    auto snapshot = SnapshotSerializer::capture(*storage);
    // 将快照序列化为 JSON 字符串
    auto json = SnapshotSerializer::toJSON(snapshot);
    
    // 验证 JSON 字符串不为空
    EXPECT_FALSE(json.empty());
    // 验证 JSON 包含 "components" 字段
    EXPECT_NE(json.find("components"), std::string::npos);
    // 验证 JSON 包含 "version" 字段
    EXPECT_NE(json.find("version"), std::string::npos);
}

/**
 * 测试用例 4：从 JSON 反序列化（正常逻辑测试）
 * 测试目标：验证能正确从 JSON 字符串恢复快照
 */
TEST_F(SnapshotTest, FromJSON) {
    // 捕获原始快照
    auto original = SnapshotSerializer::capture(*storage);
    // 将原始快照序列化为 JSON
    auto json = SnapshotSerializer::toJSON(original);
    
    // 从 JSON 反序列化为快照
    auto restored = SnapshotSerializer::fromJSON(json);
    
    // 验证恢复的快照中的组件计数与原始快照相同
    EXPECT_EQ(restored.componentCount, original.componentCount);
}

/**
 * 测试用例 5：序列化为二进制（正常逻辑测试）
 * 测试目标：验证能正确将快照序列化为二进制格式
 */
TEST_F(SnapshotTest, ToBinary) {
    // 捕获组件存储的快照
    auto snapshot = SnapshotSerializer::capture(*storage);
    // 将快照序列化为二进制格式
    auto binary = SnapshotSerializer::toBinary(snapshot);
    
    // 验证二进制数据大小大于 0
    EXPECT_GT(binary.size(), 0);
}

/**
 * 测试用例 6：从二进制反序列化（正常逻辑测试）
 * 测试目标：验证能正确从二进制数据恢复快照
 */
TEST_F(SnapshotTest, FromBinary) {
    // 捕获原始快照
    auto original = SnapshotSerializer::capture(*storage);
    // 将原始快照序列化为二进制格式
    auto binary = SnapshotSerializer::toBinary(original);
    
    // 从二进制反序列化为快照
    auto restored = SnapshotSerializer::fromBinary(binary);
    
    // 验证恢复的快照中的组件计数与原始快照相同
    EXPECT_EQ(restored.componentCount, original.componentCount);
}

/**
 * 测试用例 7：二进制往返序列化（边界测试）
 * 测试目标：验证快照能正确往返序列化并保持数据完整性
 */
TEST_F(SnapshotTest, RoundTripBinary) {
    // 创建一个按钮组件
    auto button = storage->createComponent(ComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为可见
    buttonEntry->visible = true;
    // 设置按钮为禁用状态
    buttonEntry->enabled = false;
    
    // 捕获原始快照
    auto original = SnapshotSerializer::capture(*storage);
    // 序列化为二进制
    auto binary = SnapshotSerializer::toBinary(original);
    // 从二进制反序列化
    auto restored = SnapshotSerializer::fromBinary(binary);
    
    // 验证恢复的快照中的组件计数与原始快照相同
    EXPECT_EQ(restored.componentCount, original.componentCount);
    // 验证按钮的可见性被正确恢复
    EXPECT_EQ(restored.components[1].visible, original.components[1].visible);
    // 验证按钮的启用状态被正确恢复
    EXPECT_EQ(restored.components[1].enabled, original.components[1].enabled);
}

/**
 * 测试用例 8：比较不同的快照（正常逻辑测试）
 * 测试目标：验证能正确判断两个不同的快照不相等
 */
TEST_F(SnapshotTest, CompareSnapshots) {
    // 捕获第一个快照（只有根组件）
    auto snapshot1 = SnapshotSerializer::capture(*storage);
    
    // 添加一个按钮组件
    auto button = storage->createComponent(ComponentType::Button, root);
    // 捕获第二个快照（根组件和按钮）
    auto snapshot2 = SnapshotSerializer::capture(*storage);
    
    // 验证两个快照不相等
    EXPECT_FALSE(SnapshotSerializer::compare(snapshot1, snapshot2));
}

/**
 * 测试用例 9：比较相同的快照（正常逻辑测试）
 * 测试目标：验证能正确判断两个相同的快照相等
 */
TEST_F(SnapshotTest, CompareIdenticalSnapshots) {
    // 捕获两个相同的快照
    auto snapshot1 = SnapshotSerializer::capture(*storage);
    auto snapshot2 = SnapshotSerializer::capture(*storage);
    
    // 验证两个快照相等
    EXPECT_TRUE(SnapshotSerializer::compare(snapshot1, snapshot2));
}

/**
 * 测试用例 10：计算快照差异（正常逻辑测试）
 * 测试目标：验证能正确计算两个快照之间的差异
 */
TEST_F(SnapshotTest, DiffSnapshots) {
    // 捕获第一个快照（只有根组件）
    auto snapshot1 = SnapshotSerializer::capture(*storage);
    
    // 添加一个按钮组件
    auto button = storage->createComponent(ComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 捕获第二个快照（根组件和按钮）
    auto snapshot2 = SnapshotSerializer::capture(*storage);
    
    // 计算两个快照的差异
    auto diff = SnapshotSerializer::diff(snapshot1, snapshot2);
    
    // 验证差异不为空
    EXPECT_FALSE(diff.empty());
}

/**
 * 测试用例 11：组件层级关系（正常逻辑测试）
 * 测试目标：验证快照能正确保存组件的父子层级关系
 */
TEST_F(SnapshotTest, ComponentHierarchy) {
    // 创建一个按钮组件作为根组件的子组件
    auto child = storage->createComponent(ComponentType::Button, root);
    auto* childEntry = storage->getComponent(child);
    // 设置子组件的布局区域
    childEntry->layoutResult = {50, 50, 100, 30};
    
    // 捕获快照
    auto snapshot = SnapshotSerializer::capture(*storage);
    
    bool foundChild = false;
    // 遍历快照中的所有组件
    for (const auto& comp : snapshot.components) {
        // 查找子组件
        if (comp.id == childEntry->id) {
            foundChild = true;
            // 验证子组件的父 ID 与根组件的索引相同
            EXPECT_EQ(comp.parentId, root.index);
        }
    }
    // 验证找到了子组件
    EXPECT_TRUE(foundChild);
}

} // namespace test
} // namespace aether
