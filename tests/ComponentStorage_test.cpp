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
 * 组件存储模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JComponentStorage 类的各项功能
 * - 包括组件创建、销毁、查找、遍历等核心功能
 * - 测试用例覆盖：正常逻辑、边界情况、异常处理
 */

#include "aether/ComponentStorage.h"
#include <gtest/gtest.h>
#include <vector>

namespace jaether {
namespace test {

/**
 * 组件存储测试的夹具类
 * 每个测试运行前会自动创建一个新的 JComponentStorage 实例
 */
class ComponentStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化一个新的组件存储对象
        storage = std::make_unique<JComponentStorage>();
    }

    std::unique_ptr<JComponentStorage> storage;
};

/**
 * 测试用例 1：创建根组件（正常逻辑测试）
 * 测试目标：验证可以成功创建一个根组件，并且 getRoot() 能正确返回该组件
 */
TEST_F(ComponentStorageTest, CreateRootComponent) {
    // 创建一个容器类型的根组件
    auto root = storage->createComponent(JComponentType::Container);

    // 验证返回的组件句柄有效
    ASSERT_TRUE(root.isValid());
    // 验证通过 getRoot() 获取的就是刚才创建的根组件
    EXPECT_EQ(storage->getRoot(), root);
}

/**
 * 测试用例 2：创建子组件（正常逻辑测试）
 * 测试目标：验证可以创建子组件，并且能正确设置父子关系
 */
TEST_F(ComponentStorageTest, CreateChildComponent) {
    // 先创建根组件
    auto root = storage->createComponent(JComponentType::Container);
    // 再创建一个子组件，父组件是刚才的根组件
    auto child = storage->createComponent(JComponentType::Button, root);

    // 验证子组件句柄有效
    ASSERT_TRUE(child.isValid());
    // 验证父组件句柄仍然有效
    ASSERT_TRUE(root.isValid());

    // 获取子组件的详细信息
    auto* childEntry = storage->getComponent(child);
    // 验证子组件存在
    ASSERT_NE(childEntry, nullptr);
    // 验证子组件的父索引正确设置为根组件的索引
    EXPECT_EQ(childEntry->parentIndex, root.index);
}

/**
 * 测试用例 3：销毁单个组件（正常逻辑测试）
 * 测试目标：验证可以正常销毁单个组件
 */
TEST_F(ComponentStorageTest, DestroyComponent) {
    // 创建根组件和子组件
    auto root = storage->createComponent(JComponentType::Container);
    auto child = storage->createComponent(JComponentType::Button, root);

    // 销毁子组件
    storage->destroyComponent(child);

    // 验证子组件句柄已经无效（使用storage的isValid方法）
    EXPECT_FALSE(storage->isValid(child));
}

/**
 * 测试用例 4：销毁根组件及其子组件（分支测试）
 * 测试目标：验证销毁根组件时，所有子组件也会被销毁
 */
TEST_F(ComponentStorageTest, DestroyRootChildren) {
    // 创建根组件
    auto root = storage->createComponent(JComponentType::Container);
    // 创建两个子组件
    auto child1 = storage->createComponent(JComponentType::Button, root);
    auto child2 = storage->createComponent(JComponentType::Text, root);

    // 销毁根组件
    storage->destroyComponent(root);

    // 验证根组件句柄无效（使用storage的isValid方法）
    EXPECT_FALSE(storage->isValid(root));
    // 验证子组件 1 句柄无效
    EXPECT_FALSE(storage->isValid(child1));
    // 验证子组件 2 句柄无效
    EXPECT_FALSE(storage->isValid(child2));
}

/**
 * 测试用例 5：句柄有效性检查（条件测试）
 * 测试目标：验证 isValid() 能正确判断句柄是否有效
 */
TEST_F(ComponentStorageTest, HandleValidity) {
    // 创建一个有效组件
    auto root = storage->createComponent(JComponentType::Container);
    // 创建一个无效句柄（索引为 -1，代次为 0）
    JComponentHandle invalid{-1, 0};

    // 验证根组件有效
    EXPECT_TRUE(root.isValid());
    // 验证无效句柄无效
    EXPECT_FALSE(invalid.isValid());
}

/**
 * 测试用例 6：通过 ID 查找组件（正常逻辑测试）
 * 测试目标：验证 findById() 能通过组件 ID 找到对应的组件
 */
TEST_F(ComponentStorageTest, FindById) {
    // 创建根组件
    auto root = storage->createComponent(JComponentType::Container);
    // 获取根组件的内部信息
    auto* entry = storage->getComponent(root);
    // 获取组件的唯一 ID
    JComponentId rootId = entry->id;

    // 通过 ID 查找组件
    auto found = storage->findById(rootId);
    // 验证找到的就是我们想要的根组件
    EXPECT_EQ(found, root);
}

/**
 * 测试用例 7：创建大量组件（性能与边界测试）
 * 测试目标：验证系统能正常创建和管理大量组件
 */
TEST_F(ComponentStorageTest, MultipleComponents) {
    std::vector<JComponentHandle> handles;

    // 循环创建 100 个按钮组件
    for (int i = 0; i < 100; ++i) {
        handles.push_back(storage->createComponent(JComponentType::Button));
    }

    // 验证活动组件的数量：总共创建了 100 个组件
    EXPECT_EQ(storage->activeCount(), 100);

    // 验证所有创建的组件句柄都有效
    for (auto& h : handles) {
        EXPECT_TRUE(storage->isValid(h));
    }
}

/**
 * 测试用例 8：组件遍历（正常逻辑测试）
 * 测试目标：验证 forEach() 能正确遍历所有活动组件
 */
TEST_F(ComponentStorageTest, ComponentIteration) {
    // 创建 3 个组件（容器、按钮、文本）
    storage->createComponent(JComponentType::Container);
    storage->createComponent(JComponentType::Button);
    storage->createComponent(JComponentType::Text);

    int count = 0;
    // 遍历所有组件并计数
    storage->forEach([&count](JComponentHandle) {
        count++;
    });

    // 验证遍历到的组件数量是 3
    EXPECT_EQ(count, 3);
}

/**
 * 测试用例 9：清空存储（正常逻辑测试）
 * 测试目标：验证 clear() 能清空整个存储
 */
TEST_F(ComponentStorageTest, ClearStorage) {
    // 创建两个组件
    storage->createComponent(JComponentType::Container);
    storage->createComponent(JComponentType::Button);

    // 清空整个存储
    storage->clear();

    // 验证活动组件数量为 0
    EXPECT_EQ(storage->activeCount(), 0);
}

} // namespace test
} // namespace jaether
