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
 * 四叉树模块 - 单元测试文件
 * 
 * 功能说明：
 * - 测试 JQuadTree 类的空间查询功能
 * - 包括点查询、矩形查询、组件更新等
 * - 测试用例覆盖：正常逻辑、边界情况、性能测试
 */

#include "aether/QuadTree.h"
#include <gtest/gtest.h>
#include <random>

namespace jaether {
namespace test {

/**
 * 四叉树测试的夹具类
 * 每个测试运行前会创建一个新的四叉树实例
 */
class QuadTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建一个四叉树实例，边界为 1000x1000 的正方形区域
        tree = std::make_unique<JQuadTree>(JRect{0, 0, 1000, 1000});
    }
    
    std::unique_ptr<JQuadTree> tree;  // 四叉树指针
};

/**
 * 测试用例 1：空查询（正常逻辑测试）
 * 测试目标：验证在没有组件的四叉树中查询时返回空结果
 */
TEST_F(QuadTreeTest, EmptyQuery) {
    // 在四叉树中心点 (500, 500) 进行查询
    auto result = tree->query(JPoint{500, 500});
    // 验证查询结果为空
    EXPECT_TRUE(result.empty());
}

/**
 * 测试用例 2：单个组件查询（正常逻辑测试）
 * 测试目标：验证能正确查询到单个组件
 */
TEST_F(QuadTreeTest, SingleComponent) {
    // 创建三个组件句柄
    JComponentHandle h1{0, 1};
    JComponentHandle h2{1, 1};
    JComponentHandle h3{2, 1};
    
    // 将组件添加到向量中
    std::vector<JComponentHandle> components = {h1, h2, h3};
    // 设置每个组件的边界框（位置和大小）
    std::vector<JRect> bounds = {
        {10, 10, 50, 50},    // 第一个组件：左上角 (10,10)，宽高 50x50
        {100, 100, 50, 50},  // 第二个组件：左上角 (100,100)，宽高 50x50
        {500, 500, 50, 50}   // 第三个组件：左上角 (500,500)，宽高 50x50
    };
    
    // 重建四叉树，传入组件和边界回调函数
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 查询点 (30, 30)，应该命中第一个组件（因为它在范围内）
    auto result = tree->query(JPoint{30, 30});
    // 验证结果不为空
    EXPECT_FALSE(result.empty());
    // 验证第一个被命中的组件是索引为 0 的组件
    EXPECT_EQ(result[0].index, 0);
}

/**
 * 测试用例 3：命中测试未命中（边界测试）
 * 测试目标：验证查询点不在任何组件范围内时返回空结果
 */
TEST_F(QuadTreeTest, HitTestMiss) {
    // 创建一个组件句柄
    JComponentHandle h1{0, 1};
    std::vector<JComponentHandle> components = {h1};
    // 设置组件的边界框为 (10, 10) 到 (60, 60) 的区域
    std::vector<JRect> bounds = {{10, 10, 50, 50}};
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 查询点 (500, 500)，这个点不在组件范围内
    auto result = tree->query(JPoint{500, 500});
    // 验证查询结果为空
    EXPECT_TRUE(result.empty());
}

/**
 * 测试用例 4：多个组件同时命中（正常逻辑测试）
 * 测试目标：验证当多个组件重叠时，查询能返回所有命中的组件
 */
TEST_F(QuadTreeTest, MultipleHits) {
    // 创建两个组件句柄
    JComponentHandle h1{0, 1};
    JComponentHandle h2{1, 1};
    
    std::vector<JComponentHandle> components = {h1, h2};
    // 设置两个组件的边界框，它们在 (50, 50) 到 (150, 150) 区域重叠
    std::vector<JRect> bounds = {
        {10, 10, 100, 100},   // 第一个组件：从 (10,10) 开始
        {50, 50, 100, 100}    // 第二个组件：从 (50,50) 开始，与第一个重叠
    };
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 查询点 (75, 75)，这个点在两个组件的重叠区域内
    auto result = tree->query(JPoint{75, 75});
    // 验证两个组件都被命中
    EXPECT_EQ(result.size(), 2);
}

/**
 * 测试用例 5：矩形查询（正常逻辑测试）
 * 测试目标：验证能用矩形区域查询命中的组件
 */
TEST_F(QuadTreeTest, RectQuery) {
    // 创建一个组件句柄
    JComponentHandle h1{0, 1};
    std::vector<JComponentHandle> components = {h1};
    // 设置组件的边界框
    std::vector<JRect> bounds = {{10, 10, 50, 50}};
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 用矩形区域查询，包含从 (0,0) 到 (100,100) 的区域
    auto result = tree->query(JRect{0, 0, 100, 100});
    // 验证命中了 1 个组件
    EXPECT_EQ(result.size(), 1);
}

/**
 * 测试用例 6：清空四叉树（正常逻辑测试）
 * 测试目标：验证 clear() 能正确清空四叉树中的所有数据
 */
TEST_F(QuadTreeTest, Clear) {
    // 创建一个组件句柄
    JComponentHandle h1{0, 1};
    std::vector<JComponentHandle> components = {h1};
    std::vector<JRect> bounds = {{10, 10, 50, 50}};
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 清空四叉树
    tree->clear();
    
    // 查询原来组件所在的位置
    auto result = tree->query(JPoint{30, 30});
    // 验证结果为空（因为四叉树已被清空）
    EXPECT_TRUE(result.empty());
}

/**
 * 测试用例 7：大量组件（性能测试）
 * 测试目标：验证四叉树能正确处理大量组件
 */
TEST_F(QuadTreeTest, ManyComponents) {
    std::vector<JComponentHandle> components;
    std::vector<JRect> bounds;
    
    // 创建 1000 个组件，排列成 32x32 的网格
    for (int i = 0; i < 1000; ++i) {
        // 创建组件句柄，使用索引 i
        components.push_back(JComponentHandle{i, 1});
        // 计算组件的位置，每个单元格 30x30，相邻组件间隔 30
        float x = static_cast<float>((i % 32) * 30);
        float y = static_cast<float>((i / 32) * 30);
        // 添加边界框，每个组件大小 25x25
        bounds.push_back({x, y, 25, 25});
    }
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 查询点 (50, 50)，应该能在这个网格中找到组件
    auto result = tree->query(JPoint{50, 50});
    // 验证结果数量大于 0
    EXPECT_GT(result.size(), 0);
    
    // 验证四叉树的节点数量大于等于 1
    EXPECT_GE(tree->getNodeCount(), 1);
}

/**
 * 测试用例 8：更新组件（正常逻辑测试）
 * 测试目标：验证能更新组件的位置信息
 */
TEST_F(QuadTreeTest, UpdateComponent) {
    // 创建一个组件句柄
    JComponentHandle h1{0, 1};
    std::vector<JComponentHandle> components = {h1};
    // 初始边界框在 (10, 10) 位置
    std::vector<JRect> bounds = {{10, 10, 50, 50}};
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 更新组件位置到 (500, 500)
    tree->update(h1, {500, 500, 50, 50});
    bounds[0] = {500, 500, 50, 50};
    
    // 查询原来的位置 (30, 30)，应该未命中
    auto missResult = tree->query(JPoint{30, 30});
    EXPECT_TRUE(missResult.empty());
    
    // 查询新的位置 (520, 520)，应该命中
    auto hitResult = tree->query(JPoint{520, 520});
    EXPECT_EQ(hitResult.size(), 1);
}

/**
 * 测试用例 9：压力测试（性能测试）
 * 测试目标：验证四叉树在大量组件和大量查询下的性能
 */
TEST_F(QuadTreeTest, StressTest) {
    // 使用随机数生成器创建 5000 个组件
    std::mt19937 rng(42);  // 固定种子以保证测试可重复
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);  // 均匀分布从 0 到 1000
    
    std::vector<JComponentHandle> components;
    std::vector<JRect> bounds;
    
    // 创建 5000 个组件，随机分布在 1000x1000 的区域内
    for (int i = 0; i < 5000; ++i) {
        components.push_back(JComponentHandle{i, 1});
        // 每个组件大小 20x20，位置随机
        bounds.push_back({dist(rng), dist(rng), 20.0f, 20.0f});
    }
    
    // 重建四叉树
    tree->rebuild(components, [&bounds](JComponentHandle h) {
        return bounds[h.index];
    });
    
    // 执行 100 次随机查询
    for (int i = 0; i < 100; ++i) {
        JPoint p{dist(rng), dist(rng)};
        auto result = tree->query(p);
    }
    
    // 验证四叉树的节点数量大于 0
    EXPECT_GT(tree->getNodeCount(), 0);
}

} // namespace test
} // namespace jaether
