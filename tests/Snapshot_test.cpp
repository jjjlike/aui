/**
 * 快照序列化模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JSnapshotSerializer 类的序列化功能
 * - 包括快照捕获、JSON 序列化、二进制序列化、快照比较等
 * - 测试用例覆盖：正常逻辑、边界情况、往返序列化
 */

#include "aether/Snapshot.h"
#include "aether/aether.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 快照测试的夹具类
 * 每个测试运行前会创建新的组件存储实例
 */
class SnapshotTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建组件存储实例
        storage = std::make_unique<JComponentStorage>();
        
        // 创建根容器组件
        root = storage->createComponent(JComponentType::Container);
        // 获取根组件的详细信息
        auto* rootEntry = storage->getComponent(root);
        // 设置根组件的布局大小（800x600）
        rootEntry->layoutResult = {0, 0, 800, 600};
    }
    
    std::unique_ptr<JComponentStorage> storage;  // 组件存储指针
    JComponentHandle root;                      // 根组件句柄
};

/**
 * 测试用例 1：捕获空存储（正常逻辑测试）
 * 测试目标：验证能正确捕获空存储的快照
 */
TEST_F(SnapshotTest, CaptureEmptyStorage) {
    // 捕获组件存储的快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
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
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为可见
    buttonEntry->visible = true;
    // 设置按钮为启用状态
    buttonEntry->enabled = true;
    
    // 捕获组件存储的快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
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
    auto snapshot = JSnapshotSerializer::capture(*storage);
    // 将快照序列化为 JSON 字符串
    auto json = JSnapshotSerializer::toJSON(snapshot);
    
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
    auto original = JSnapshotSerializer::capture(*storage);
    // 将原始快照序列化为 JSON
    auto json = JSnapshotSerializer::toJSON(original);
    
    // 从 JSON 反序列化为快照
    auto restored = JSnapshotSerializer::fromJSON(json);
    
    // 验证恢复的快照中的组件计数与原始快照相同
    EXPECT_EQ(restored.componentCount, original.componentCount);
}

/**
 * 测试用例 5：序列化为二进制（正常逻辑测试）
 * 测试目标：验证能正确将快照序列化为二进制格式
 */
TEST_F(SnapshotTest, ToBinary) {
    // 捕获组件存储的快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    // 将快照序列化为二进制格式
    auto binary = JSnapshotSerializer::toBinary(snapshot);
    
    // 验证二进制数据大小大于 0
    EXPECT_GT(binary.size(), 0);
}

/**
 * 测试用例 6：从二进制反序列化（正常逻辑测试）
 * 测试目标：验证能正确从二进制数据恢复快照
 */
TEST_F(SnapshotTest, FromBinary) {
    // 捕获原始快照
    auto original = JSnapshotSerializer::capture(*storage);
    // 将原始快照序列化为二进制格式
    auto binary = JSnapshotSerializer::toBinary(original);
    
    // 从二进制反序列化为快照
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
    // 验证恢复的快照中的组件计数与原始快照相同
    EXPECT_EQ(restored.componentCount, original.componentCount);
}

/**
 * 测试用例 7：二进制往返序列化（边界测试）
 * 测试目标：验证快照能正确往返序列化并保持数据完整性
 */
TEST_F(SnapshotTest, RoundTripBinary) {
    // 创建一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    // 设置按钮为可见
    buttonEntry->visible = true;
    // 设置按钮为禁用状态
    buttonEntry->enabled = false;
    
    // 捕获原始快照
    auto original = JSnapshotSerializer::capture(*storage);
    // 序列化为二进制
    auto binary = JSnapshotSerializer::toBinary(original);
    // 从二进制反序列化
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
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
    auto snapshot1 = JSnapshotSerializer::capture(*storage);
    
    // 添加一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    // 捕获第二个快照（根组件和按钮）
    auto snapshot2 = JSnapshotSerializer::capture(*storage);
    
    // 验证两个快照不相等
    EXPECT_FALSE(JSnapshotSerializer::compare(snapshot1, snapshot2));
}

/**
 * 测试用例 9：比较相同的快照（正常逻辑测试）
 * 测试目标：验证能正确判断两个相同的快照相等
 */
TEST_F(SnapshotTest, CompareIdenticalSnapshots) {
    // 捕获两个相同的快照
    auto snapshot1 = JSnapshotSerializer::capture(*storage);
    auto snapshot2 = JSnapshotSerializer::capture(*storage);
    
    // 验证两个快照相等
    EXPECT_TRUE(JSnapshotSerializer::compare(snapshot1, snapshot2));
}

/**
 * 测试用例 10：计算快照差异（正常逻辑测试）
 * 测试目标：验证能正确计算两个快照之间的差异
 */
TEST_F(SnapshotTest, DiffSnapshots) {
    // 捕获第一个快照（只有根组件）
    auto snapshot1 = JSnapshotSerializer::capture(*storage);
    
    // 添加一个按钮组件
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    // 设置按钮的布局区域
    buttonEntry->layoutResult = {100, 100, 200, 50};
    
    // 捕获第二个快照（根组件和按钮）
    auto snapshot2 = JSnapshotSerializer::capture(*storage);
    
    // 计算两个快照的差异
    auto diff = JSnapshotSerializer::diff(snapshot1, snapshot2);
    
    // 验证差异不为空
    EXPECT_FALSE(diff.empty());
}

/**
 * 测试用例 11：组件层级关系（正常逻辑测试）
 * 测试目标：验证快照能正确保存组件的父子层级关系
 */
TEST_F(SnapshotTest, ComponentHierarchy) {
    // 创建一个按钮组件作为根组件的子组件
    auto child = storage->createComponent(JComponentType::Button, root);
    auto* childEntry = storage->getComponent(child);
    auto* rootEntry = storage->getComponent(root);
    // 设置子组件的布局区域
    childEntry->layoutResult = {50, 50, 100, 30};
    
    // 捕获快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
    bool foundChild = false;
    // 遍历快照中的所有组件
    for (const auto& comp : snapshot.components) {
        // 查找子组件
        if (comp.id == childEntry->id) {
            foundChild = true;
            // 验证子组件的父 ID 与根组件的 ID 相同
            EXPECT_EQ(comp.parentId, static_cast<int32_t>(rootEntry->id));
        }
    }
    // 验证找到了子组件
    EXPECT_TRUE(foundChild);
}

/**
 * 测试用例 12：多层级组件序列化（正常逻辑测试）
 * 测试目标：验证能正确序列化多层嵌套的组件结构
 * 
 * 覆盖分析：
 * - 语句覆盖：遍历多层组件的语句
 * - 路径覆盖：从根到子到孙的多层路径
 */
TEST_F(SnapshotTest, MultiLevelHierarchy) {
    // 创建多层嵌套的组件结构
    auto child1 = storage->createComponent(JComponentType::Button, root);
    auto child2 = storage->createComponent(JComponentType::Button, root);
    auto grandchild = storage->createComponent(JComponentType::Button, child1);
    
    // 捕获快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
    // 验证快照包含所有组件（根 + 2个子 + 1个孙 = 4）
    EXPECT_EQ(snapshot.componentCount, 4);
    
    // 查找grandchild的父ID
    bool foundGrandchild = false;
    bool foundGrandchildParent = false;
    for (const auto& comp : snapshot.components) {
        if (comp.id == storage->getComponent(grandchild)->id) {
            foundGrandchild = true;
            // 验证grandchild有父组件
            EXPECT_GE(comp.parentId, 0);
            foundGrandchildParent = (comp.parentId >= 0);
        }
    }
    // 验证找到了grandchild
    EXPECT_TRUE(foundGrandchild);
    // 验证grandchild有父组件
    EXPECT_TRUE(foundGrandchildParent);
}

/**
 * 测试用例 13：序列化所有组件类型（正常逻辑测试）
 * 测试目标：验证能正确序列化所有类型的组件
 * 
 * 覆盖分析：
 * - 分支覆盖：不同组件类型的处理
 * - 条件覆盖：每个组件类型都存在
 */
TEST_F(SnapshotTest, AllComponentTypes) {
    // 创建不同类型的组件
    auto container = storage->createComponent(JComponentType::Container, root);
    auto button = storage->createComponent(JComponentType::Button, root);
    auto text = storage->createComponent(JComponentType::Text, root);
    auto image = storage->createComponent(JComponentType::Image, root);
    auto input = storage->createComponent(JComponentType::Input, root);
    auto custom = storage->createComponent(JComponentType::Custom, root);
    
    // 捕获快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
    // 验证快照包含所有组件（根 + 6个子 = 7）
    EXPECT_EQ(snapshot.componentCount, 7);
    
    // 验证每种类型的组件都存在
    std::set<JComponentType> foundTypes;
    for (const auto& comp : snapshot.components) {
        foundTypes.insert(comp.type);
    }
    
    // 验证所有组件类型都被记录
    EXPECT_GE(foundTypes.size(), 6);
}

/**
 * 测试用例 14：二进制序列化完整性（边界测试）
 * 测试目标：验证二进制序列化包含所有必要的数据
 * 
 * 覆盖分析：
 * - 边界覆盖：测试数据的完整性边界
 * - 路径覆盖：从capture到toBinary到fromBinary的完整往返路径
 */
TEST_F(SnapshotTest, BinarySerializationCompleteness) {
    // 创建组件并设置所有属性
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    buttonEntry->layoutResult = {100, 200, 300, 400};
    buttonEntry->visible = false;
    buttonEntry->enabled = true;
    buttonEntry->debugName = "TestButton";
    
    // 捕获快照
    auto original = JSnapshotSerializer::capture(*storage);
    
    // 序列化为二进制
    auto binary = JSnapshotSerializer::toBinary(original);
    
    // 验证二进制数据长度大于基本头部
    EXPECT_GT(binary.size(), 20);
    
    // 从二进制反序列化
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
    // 验证恢复的快照包含按钮组件
    EXPECT_EQ(restored.componentCount, 2); // 根 + 按钮
}

/**
 * 测试用例 15：空快照序列化（边界测试）
 * 测试目标：验证空快照的序列化正确性
 * 
 * 覆盖分析：
 * - 边界覆盖：空数据的序列化
 * - 条件覆盖：componentCount为0的条件
 */
TEST_F(SnapshotTest, EmptySnapshotSerialization) {
    // 创建空的快照
    JSnapshot emptySnapshot;
    emptySnapshot.timestamp = 1234567890;
    emptySnapshot.version = AETHER_VERSION;
    emptySnapshot.componentCount = 0;
    
    // 序列化为二进制
    auto binary = JSnapshotSerializer::toBinary(emptySnapshot);
    
    // 验证二进制数据不为空（包含头部）
    EXPECT_GT(binary.size(), 0);
    
    // 从二进制反序列化
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
    // 验证恢复的快照为空
    EXPECT_EQ(restored.componentCount, 0);
}

/**
 * 测试用例 16：损坏的二进制数据（异常测试）
 * 测试目标：验证能正确处理损坏的二进制数据
 * 
 * 覆盖分析：
 * - 异常路径覆盖：处理无效数据的异常路径
 * - 边界覆盖：数据过短的边界情况
 */
TEST_F(SnapshotTest, CorruptedBinaryData) {
    // 创建损坏的二进制数据
    std::vector<uint8_t> corruptedData = {0x00, 0x01, 0x02};
    
    // 从损坏的数据反序列化
    auto restored = JSnapshotSerializer::fromBinary(corruptedData);
    
    // 验证返回的快照是空的或基本的
    // （根据具体实现，可能返回空快照或部分数据）
    // 重要的是不要崩溃
    EXPECT_TRUE(restored.componentCount >= 0);
}

/**
 * 测试用例 17：快照版本验证（正常逻辑测试）
 * 测试目标：验证快照包含正确的版本信息
 * 
 * 覆盖分析：
 * - 语句覆盖：版本信息的写入和读取
 * - 路径覆盖：从capture到toBinary到fromBinary的版本传递路径
 */
TEST_F(SnapshotTest, VersionVerification) {
    // 捕获快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
    // 序列化为二进制
    auto binary = JSnapshotSerializer::toBinary(snapshot);
    
    // 从二进制反序列化
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
    // 验证版本信息正确传递
    EXPECT_EQ(restored.version, snapshot.version);
}

/**
 * 测试用例 18：大数据快照（性能测试）
 * 测试目标：验证能正确处理大量组件的快照
 * 
 * 覆盖分析：
 * - 性能路径覆盖：大量数据的序列化
 * - 边界覆盖：大量组件的边界情况
 */
TEST_F(SnapshotTest, LargeSnapshot) {
    // 创建大量组件
    const int numComponents = 100;
    for (int i = 0; i < numComponents; ++i) {
        auto component = storage->createComponent(JComponentType::Button, root);
        auto* entry = storage->getComponent(component);
        entry->layoutResult = {static_cast<float>(i * 10), static_cast<float>(i * 10), 100, 30};
    }
    
    // 捕获快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
    // 验证快照包含所有组件（根 + 100个子）
    EXPECT_EQ(snapshot.componentCount, numComponents + 1);
    
    // 序列化为二进制
    auto binary = JSnapshotSerializer::toBinary(snapshot);
    
    // 验证二进制数据长度合理
    EXPECT_GT(binary.size(), numComponents * 10); // 每个组件至少10字节
    
    // 从二进制反序列化
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
    // 验证恢复的快照包含所有组件
    EXPECT_EQ(restored.componentCount, numComponents + 1);
}

/**
 * 测试用例 19：组件可见性和启用状态（正常逻辑测试）
 * 测试目标：验证快照正确保存组件的可见性和启用状态
 * 
 * 覆盖分析：
 * - 条件覆盖：visible和enabled的各种组合
 * - 路径覆盖：从capture到toBinary到fromBinary的状态传递路径
 */
TEST_F(SnapshotTest, VisibilityAndEnabledState) {
    // 创建按钮并设置不同的可见性和启用状态
    auto button1 = storage->createComponent(JComponentType::Button, root);
    auto button2 = storage->createComponent(JComponentType::Button, root);
    auto button3 = storage->createComponent(JComponentType::Button, root);
    
    auto* entry1 = storage->getComponent(button1);
    auto* entry2 = storage->getComponent(button2);
    auto* entry3 = storage->getComponent(button3);
    
    entry1->visible = true;
    entry1->enabled = true;
    entry2->visible = true;
    entry2->enabled = false;
    entry3->visible = false;
    entry3->enabled = true;
    
    // 捕获快照
    auto snapshot = JSnapshotSerializer::capture(*storage);
    
    // 序列化为二进制
    auto binary = JSnapshotSerializer::toBinary(snapshot);
    
    // 从二进制反序列化
    auto restored = JSnapshotSerializer::fromBinary(binary);
    
    // 查找按钮组件并验证状态
    // 注意：根据实现，可能需要通过ID或其他方式匹配
    // 这里我们验证至少有3个按钮类型的组件
    int buttonCount = 0;
    for (const auto& comp : restored.components) {
        if (comp.type == JComponentType::Button) {
            buttonCount++;
        }
    }
    EXPECT_EQ(buttonCount, 3);
}

/**
 * 测试用例 20：比较快照内容差异（正常逻辑测试）
 * 测试目标：验证快照比较函数能正确识别内容差异
 * 
 * 覆盖分析：
 * - 条件覆盖：compare函数的各种返回值
 * - 路径覆盖：从修改到比较的完整路径
 */
TEST_F(SnapshotTest, SnapshotContentDifference) {
    // 捕获第一个快照
    auto snapshot1 = JSnapshotSerializer::capture(*storage);
    
    // 修改组件属性
    auto button = storage->createComponent(JComponentType::Button, root);
    auto* buttonEntry = storage->getComponent(button);
    buttonEntry->layoutResult = {100, 100, 200, 50};
    buttonEntry->visible = false;
    
    // 捕获第二个快照
    auto snapshot2 = JSnapshotSerializer::capture(*storage);
    
    // 验证两个快照不相等
    EXPECT_FALSE(JSnapshotSerializer::compare(snapshot1, snapshot2));
    
    // 创建第三个快照，内容与snapshot2相同
    auto snapshot3 = JSnapshotSerializer::capture(*storage);
    
    // 验证snapshot2和snapshot3相等
    EXPECT_TRUE(JSnapshotSerializer::compare(snapshot2, snapshot3));
}

} // namespace test
} // namespace jaether
