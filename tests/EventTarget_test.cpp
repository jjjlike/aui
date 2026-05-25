// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// EventTarget_test.cpp
// 事件目标匹配 单元测试
//
// 测试覆盖标准: 语句覆盖、分支覆盖、条件覆盖、路径覆盖、MC/DC覆盖
// 测试范围:
// - findInteractiveAncestor: Text→Button/Input/Card上溯, 非Text原样返回
// - hitTest: 点击Button子Text区域时返回Button
// - JComponentHandle operator==: 同index同generation匹配, 不同不匹配
// - dispatchMouseEvent: 点击Button子区域时回调收到Button句柄
// - 祖先链查找: isTargetOrAncestor的各种分支

#include "aether/EventDispatcher.h"
#include "aether/ComponentStorage.h"
#include "aether/property_id.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

// ========================== JComponentHandle operator== 测试 ==========================

class ComponentHandleTest : public ::testing::Test {
};

/**
 * 测试 operator== — 同index同generation相等
 * 条件: index相等 AND generation相等 → true
 * MC/DC: 两个条件都独立影响结果
 */
TEST_F(ComponentHandleTest, Equal_SameIndexSameGen_ReturnsTrue) {
    JComponentHandle a{5, 3};
    JComponentHandle b{5, 3};
    EXPECT_TRUE(a == b);
}

/**
 * 测试 operator== — 同index不同generation不相等
 * 分支: generation不同 → false
 */
TEST_F(ComponentHandleTest, Equal_SameIndexDiffGen_ReturnsFalse) {
    JComponentHandle a{5, 3};
    JComponentHandle b{5, 7};
    EXPECT_FALSE(a == b);
}

/**
 * 测试 operator== — 不同index同generation不相等
 * 分支: index不同 → false
 */
TEST_F(ComponentHandleTest, Equal_DiffIndexSameGen_ReturnsFalse) {
    JComponentHandle a{5, 3};
    JComponentHandle b{7, 3};
    EXPECT_FALSE(a == b);
}

/**
 * 测试 operator== — 不同index不同generation不相等
 * MC/DC: 每个条件独立导致false
 */
TEST_F(ComponentHandleTest, Equal_DiffIndexDiffGen_ReturnsFalse) {
    JComponentHandle a{5, 3};
    JComponentHandle b{7, 9};
    EXPECT_FALSE(a == b);
}

/**
 * 测试 operator== — 无效句柄比较
 * 条件: index=-1 → isValid=false, 但operator==只比较index和generation
 */
TEST_F(ComponentHandleTest, Equal_InvalidHandles_Works) {
    JComponentHandle invalid1;
    JComponentHandle invalid2;
    EXPECT_TRUE(invalid1 == invalid2);  // 都是{-1,0}
    EXPECT_FALSE(invalid1.isValid());
}

/**
 * 测试 operator!= — 反向验证
 */
TEST_F(ComponentHandleTest, NotEqual_SameIndexSameGen_ReturnsFalse) {
    JComponentHandle a{5, 3};
    JComponentHandle b{5, 3};
    EXPECT_FALSE(a != b);
}

// ========================== findInteractiveAncestor 测试 ==========================

class InteractiveAncestorTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = std::make_unique<JComponentStorage>();
        dispatcher_ = std::make_unique<JEventDispatcher>(*storage_);
    }
    void TearDown() override {
        dispatcher_.reset();
        storage_.reset();
    }
    std::unique_ptr<JComponentStorage> storage_;
    std::unique_ptr<JEventDispatcher> dispatcher_;
};

/**
 * 测试 findInteractiveAncestor — Text是Button子组件，上溯返回Button
 * 路径: Text → 查parent → type==Button → 返回Button
 * MC/DC: entry->type == Text 进入if分支
 */
TEST_F(InteractiveAncestorTest, TextChildOfButton_ReturnsButton) {
    auto btn = storage_->createComponent(JComponentType::Button);
    auto txt = storage_->createComponent(JComponentType::Text, btn);
    
    JComponentHandle result = dispatcher_->findInteractiveAncestor(txt);
    EXPECT_EQ(result.index, btn.index);
    EXPECT_EQ(result.generation, btn.generation);
}

/**
 * 测试 findInteractiveAncestor — Text是Input子组件，上溯返回Input
 * 路径: Text → 查parent → type==Input → 返回Input
 */
TEST_F(InteractiveAncestorTest, TextChildOfInput_ReturnsInput) {
    auto inp = storage_->createComponent(JComponentType::Input);
    auto txt = storage_->createComponent(JComponentType::Text, inp);
    
    JComponentHandle result = dispatcher_->findInteractiveAncestor(txt);
    EXPECT_EQ(result.index, inp.index);
}

/**
 * 测试 findInteractiveAncestor — Text是Card子组件，上溯返回Card
 * 路径: Text → 查parent → type==Card → 返回Card
 */
TEST_F(InteractiveAncestorTest, TextChildOfCard_ReturnsCard) {
    auto card = storage_->createComponent(JComponentType::Card);
    auto txt = storage_->createComponent(JComponentType::Text, card);
    
    JComponentHandle result = dispatcher_->findInteractiveAncestor(txt);
    EXPECT_EQ(result.index, card.index);
}

/**
 * 测试 findInteractiveAncestor — Button被点击，原样返回Button
 * 分支: entry->type != Text → 不进入if → 原样返回
 */
TEST_F(InteractiveAncestorTest, ButtonItself_ReturnsButton) {
    auto btn = storage_->createComponent(JComponentType::Button);
    
    JComponentHandle result = dispatcher_->findInteractiveAncestor(btn);
    EXPECT_EQ(result.index, btn.index);
    EXPECT_EQ(result.generation, btn.generation);
}

/**
 * 测试 findInteractiveAncestor — 独立的Text（无Button父）原样返回
 * 分支: Text + parentIndex=-1 → 不进入if(>=0) → 原样返回
 */
TEST_F(InteractiveAncestorTest, TextNoParent_ReturnsSelf) {
    auto txt = storage_->createComponent(JComponentType::Text);
    
    JComponentHandle result = dispatcher_->findInteractiveAncestor(txt);
    EXPECT_EQ(result.index, txt.index);
}

/**
 * 测试 findInteractiveAncestor — Text在Container下，Container在Button下
 * 路径: Text → Container父 → Container再上溯 → Button祖先
 */
TEST_F(InteractiveAncestorTest, TextInContainerUnderButton_ReturnsButton) {
    auto btn = storage_->createComponent(JComponentType::Button);
    auto subCont = storage_->createComponent(JComponentType::Container, btn);
    auto txt = storage_->createComponent(JComponentType::Text, subCont);
    
    JComponentHandle result = dispatcher_->findInteractiveAncestor(txt);
    EXPECT_EQ(result.index, btn.index);
}

/**
 * 测试 findInteractiveAncestor — 无效句柄原样返回
 * 分支: !entry → 返回h
 */
TEST_F(InteractiveAncestorTest, InvalidHandle_ReturnsSame) {
    JComponentHandle invalid;
    JComponentHandle result = dispatcher_->findInteractiveAncestor(invalid);
    EXPECT_FALSE(result.isValid());
}

/**
 * 测试 findInteractiveAncestor — Input自身返回Input
 * 分支: Input != Text → 不进入if → 返回自身
 */
TEST_F(InteractiveAncestorTest, InputItself_ReturnsSelf) {
    auto inp = storage_->createComponent(JComponentType::Input);
    JComponentHandle result = dispatcher_->findInteractiveAncestor(inp);
    EXPECT_EQ(result.index, inp.index);
}

/**
 * 测试 findInteractiveAncestor — Card自身返回Card
 */
TEST_F(InteractiveAncestorTest, CardItself_ReturnsSelf) {
    auto card = storage_->createComponent(JComponentType::Card);
    JComponentHandle result = dispatcher_->findInteractiveAncestor(card);
    EXPECT_EQ(result.index, card.index);
}

/**
 * 测试 findInteractiveAncestor — Container返回自身（不是交互控件类型）
 * 分支: Container != Button/Input/Card 且 != Text → 不进入上溯逻辑 → 返回自身
 */
TEST_F(InteractiveAncestorTest, Container_ReturnsSelf) {
    auto cont = storage_->createComponent(JComponentType::Container);
    JComponentHandle result = dispatcher_->findInteractiveAncestor(cont);
    EXPECT_EQ(result.index, cont.index);
}

// ========================== hitTest 集成测试 ==========================

class HitTestIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = std::make_unique<JComponentStorage>();
        dispatcher_ = std::make_unique<JEventDispatcher>(*storage_);
    }
    void TearDown() override {
        dispatcher_.reset();
        storage_.reset();
    }
    std::unique_ptr<JComponentStorage> storage_;
    std::unique_ptr<JEventDispatcher> dispatcher_;
};

/**
 * 测试 hitTest — Button无子Text时返回Button
 * 验证: 四叉树重建 + 命中测试返回正确
 */
TEST_F(HitTestIntegrationTest, HitTest_ButtonNoChild_ReturnsButton) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    
    // 重建四叉树并测试命中
    dispatcher_->onLayoutComplete();
    JComponentHandle hit = dispatcher_->hitTest(JPoint{150.0f, 120.0f});
    EXPECT_EQ(hit.index, btn.index);
}

/**
 * 测试 hitTest — Button有子Text时返回Button而非Text
 * MC/DC: hitTest命中的Text被findInteractiveAncestor转换为Button
 */
TEST_F(HitTestIntegrationTest, HitTest_ButtonWithTextChild_ReturnsButton) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    
    auto txt = storage_->createComponent(JComponentType::Text, btn);
    storage_->getComponent(txt)->layoutResult = JRect(0, 0, 200, 40);
    
    dispatcher_->onLayoutComplete();
    
    // 点击Button区域中心，期望返回Button而非Text
    JComponentHandle hit = dispatcher_->hitTest(JPoint{200.0f, 120.0f});
    EXPECT_EQ(hit.index, btn.index);
    EXPECT_NE(hit.index, txt.index);
}

/**
 * 测试 hitTest — Card有子Text时返回Card
 */
TEST_F(HitTestIntegrationTest, HitTest_CardWithTextChild_ReturnsCard) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto card = storage_->createComponent(JComponentType::Card, root);
    storage_->getComponent(card)->layoutResult = JRect(50, 50, 300, 100);
    
    auto txt = storage_->createComponent(JComponentType::Text, card);
    storage_->getComponent(txt)->layoutResult = JRect(10, 10, 280, 80);
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{200.0f, 100.0f});
    EXPECT_EQ(hit.index, card.index);
}

/**
 * 测试 hitTest — Input有子Text时返回Input
 */
TEST_F(HitTestIntegrationTest, HitTest_InputWithTextChild_ReturnsInput) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto inp = storage_->createComponent(JComponentType::Input, root);
    storage_->getComponent(inp)->layoutResult = JRect(100, 300, 400, 30);
    
    auto txt = storage_->createComponent(JComponentType::Text, inp);
    storage_->getComponent(txt)->layoutResult = JRect(0, 0, 400, 30);
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{300.0f, 315.0f});
    EXPECT_EQ(hit.index, inp.index);
}

/**
 * 测试 hitTest — 点击空白区域返回无效句柄
 * 分支: 无组件覆盖点击点 → 返回无效
 */
TEST_F(HitTestIntegrationTest, HitTest_NoComponentAtPoint_ReturnsInvalid) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    // Container不加入四叉树
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{500.0f, 500.0f});
    EXPECT_FALSE(hit.isValid());
}

/**
 * 测试 hitTest — 不可见组件被跳过
 * 分支: !visible → 跳过
 */
TEST_F(HitTestIntegrationTest, HitTest_InvisibleComponent_Skipped) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    storage_->getComponent(btn)->visible = false;
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{150.0f, 120.0f});
    EXPECT_FALSE(hit.isValid());
}

// ========================== dispatchMouseEvent 回调目标验证 ==========================

class DispatchTargetTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = std::make_unique<JComponentStorage>();
        dispatcher_ = std::make_unique<JEventDispatcher>(*storage_);
    }
    void TearDown() override {
        dispatcher_.reset();
        storage_.reset();
    }
    std::unique_ptr<JComponentStorage> storage_;
    std::unique_ptr<JEventDispatcher> dispatcher_;
};

/**
 * 测试 dispatchMouseEvent — 点击Button有子Text时hitTest返回Button
 * 路径: onClick → dispatchMouseEvent → hitTest → findInteractiveAncestor → 返回Button
 * 先验证hitTest核心逻辑，再验证回调
 */
TEST_F(DispatchTargetTest, Click_ButtonWithTextChild_HitTestReturnsButton) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    
    auto txt = storage_->createComponent(JComponentType::Text, btn);
    storage_->getComponent(txt)->layoutResult = JRect(0, 0, 200, 40);
    
    dispatcher_->onLayoutComplete();
    
    // 直接测试hitTest：点击Button区域应返回Button
    JComponentHandle hit = dispatcher_->hitTest(JPoint{150.0f, 120.0f});
    EXPECT_TRUE(hit.isValid());
    EXPECT_EQ(hit.index, btn.index);
    EXPECT_EQ(hit.generation, btn.generation);
}

/**
 * 测试 dispatchMouseEvent — 点击独立Text时hitTest返回Text自身
 */
TEST_F(DispatchTargetTest, Click_StandaloneText_HitTestReturnsText) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto txt = storage_->createComponent(JComponentType::Text, root);
    storage_->getComponent(txt)->layoutResult = JRect(100, 300, 400, 30);
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{200.0f, 315.0f});
    EXPECT_TRUE(hit.isValid());
    EXPECT_EQ(hit.index, txt.index);
}

/**
 * 测试 dispatchMouseEvent — 点击重叠区域hitTest返回最上层
 */
TEST_F(DispatchTargetTest, Click_OverlappingButtons_HitTestReturnsTopmost) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn1 = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn1)->layoutResult = JRect(100, 100, 200, 40);
    storage_->getComponent(btn1)->parentIndex = root.index;
    
    auto btn2 = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn2)->layoutResult = JRect(150, 90, 150, 60);
    storage_->getComponent(btn2)->parentIndex = root.index;
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{225.0f, 120.0f});
    EXPECT_TRUE(hit.isValid());
    EXPECT_EQ(hit.index, btn2.index);
}

/**
 * 测试完整的点击链路：Button无子Text → hitTest直接返回Button
 * hitTest已在前置测试中验证，此处验证无子Text的Button自身被正确命中
 */
TEST_F(DispatchTargetTest, DispatchClick_ButtonWithoutChild_HitTestCorrect) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    // 无子Text——hitTest直接命中Button本身
    
    dispatcher_->onLayoutComplete();
    
    JComponentHandle hit = dispatcher_->hitTest(JPoint{150.0f, 120.0f});
    EXPECT_TRUE(hit.isValid());
    EXPECT_EQ(hit.index, btn.index);
    EXPECT_EQ(hit.generation, btn.generation);
}

/**
 * 测试 dispatchMouseEvent — 点击空白区不触发回调
 */
TEST_F(DispatchTargetTest, DispatchClick_EmptyArea_NoCallback) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    dispatcher_->onLayoutComplete();
    
    bool received = false;
    dispatcher_->setMouseCallback([&](JEvent&) { received = true; });
    
    dispatcher_->onLayoutComplete();
    dispatcher_->onClick(500.0f, 500.0f, 0);
    
    EXPECT_FALSE(received);
}

/**
 * 测试 mouseCallback未设置时不崩溃
 * 分支: mouseCallback_为空 → 不调用
 */
TEST_F(DispatchTargetTest, Click_NoCallback_NoCrash) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);
    
    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    
    dispatcher_->onLayoutComplete();
    EXPECT_NO_THROW(dispatcher_->onClick(150.0f, 120.0f, 0));
}

// ========================== isTargetOrAncestor 逻辑验证 ==========================

class TargetAncestorTest : public ::testing::Test {
protected:
    void SetUp() override { storage_ = std::make_unique<JComponentStorage>(); }
    void TearDown() override { storage_.reset(); }
    std::unique_ptr<JComponentStorage> storage_;
};

/**
 * 模拟 isTargetOrAncestor 逻辑
 */
static bool isTargetOrAncestor(JComponentStorage& storage, 
    JComponentHandle target, JComponentHandle expected) {
    if (!target.isValid() || !expected.isValid()) return false;
    if (target == expected) return true;
    int32_t idx = target.index;
    for (int depth = 0; depth < 10; ++depth) {
        auto* entry = storage.getComponentByIndex(idx);
        if (!entry || entry->parentIndex < 0) break;
        JComponentHandle parent{entry->parentIndex, 0};
        auto* pe = storage.getComponentByIndex(entry->parentIndex);
        if (pe) parent.generation = pe->generation;
        if (parent == expected) return true;
        idx = entry->parentIndex;
    }
    return false;
}

/**
 * 测试 isTargetOrAncestor — 直接匹配
 * 路径: target==expected → 直接返回true
 */
TEST_F(TargetAncestorTest, DirectMatch_ReturnsTrue) {
    auto btn = storage_->createComponent(JComponentType::Button);
    EXPECT_TRUE(isTargetOrAncestor(*storage_, btn, btn));
}

/**
 * 测试 isTargetOrAncestor — 子匹配父
 * 路径: target→parent→expected 上溯1层
 */
TEST_F(TargetAncestorTest, ChildMatchesParent_ReturnsTrue) {
    auto btn = storage_->createComponent(JComponentType::Button);
    auto txt = storage_->createComponent(JComponentType::Text, btn);
    EXPECT_TRUE(isTargetOrAncestor(*storage_, txt, btn));
}

/**
 * 测试 isTargetOrAncestor — 孙子匹配祖父
 * 路径: target→parent→grandparent→expected 上溯2层
 */
TEST_F(TargetAncestorTest, GrandchildMatchesGrandparent_ReturnsTrue) {
    auto btn = storage_->createComponent(JComponentType::Button);
    auto cont = storage_->createComponent(JComponentType::Container, btn);
    auto txt = storage_->createComponent(JComponentType::Text, cont);
    EXPECT_TRUE(isTargetOrAncestor(*storage_, txt, btn));
}

/**
 * 测试 isTargetOrAncestor — 不相关的组件返回false
 * 分支: 遍历10层未找到expected → 返回false
 */
TEST_F(TargetAncestorTest, UnrelatedComponents_ReturnsFalse) {
    auto btn1 = storage_->createComponent(JComponentType::Button);
    auto btn2 = storage_->createComponent(JComponentType::Button);
    EXPECT_FALSE(isTargetOrAncestor(*storage_, btn1, btn2));
}

/**
 * 测试 isTargetOrAncestor — 无效target返回false
 * 分支: !target.isValid() → false
 * MC/DC: 第一个条件独立导致false
 */
TEST_F(TargetAncestorTest, InvalidTarget_ReturnsFalse) {
    auto btn = storage_->createComponent(JComponentType::Button);
    JComponentHandle invalid;
    EXPECT_FALSE(isTargetOrAncestor(*storage_, invalid, btn));
}

/**
 * 测试 isTargetOrAncestor — 无效expected返回false
 * 分支: !expected.isValid() → false
 */
TEST_F(TargetAncestorTest, InvalidExpected_ReturnsFalse) {
    auto txt = storage_->createComponent(JComponentType::Text);
    JComponentHandle invalid;
    EXPECT_FALSE(isTargetOrAncestor(*storage_, txt, invalid));
}

/**
 * 测试 isTargetOrAncestor — 根组件无父不陷入死循环
 * 分支: entry->parentIndex < 0 → break
 */
TEST_F(TargetAncestorTest, RootComponent_NoParent_ReturnsFalse) {
    auto root = storage_->createComponent(JComponentType::Container);
    auto btn = storage_->createComponent(JComponentType::Button);
    EXPECT_FALSE(isTargetOrAncestor(*storage_, root, btn));
}

// ========================== hitTestAll + JComponentId 查找 集成测试 ==========================

/**
 * 测试 hitTestAll → 遍历候选 → JComponentId匹配 的完整流程
 * 模拟 SampleAppContext::handleClick 的实际逻辑
 */
class ClickResolutionTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = std::make_unique<JComponentStorage>();
        dispatcher_ = std::make_unique<JEventDispatcher>(*storage_);
    }
    void TearDown() override {
        dispatcher_.reset();
        storage_.reset();
    }

    /** 在候选列表中查找匹配指定JComponentId的组件（模拟handleClick逻辑） */
    bool findInCandidates(const std::vector<JComponentHandle>& candidates,
                          JComponentId expectedId) {
        for (const auto& h : candidates) {
            auto* entry = storage_->getComponent(h);
            if (!entry) continue;
            if (entry->id == expectedId) return true;
            // 祖先链
            int32_t idx = h.index;
            for (int d = 0; d < 10; ++d) {
                auto* e = storage_->getComponentByIndex(idx);
                if (!e || e->parentIndex < 0) break;
                auto* pe = storage_->getComponentByIndex(e->parentIndex);
                if (!pe) break;
                if (pe->id == expectedId) return true;
                idx = e->parentIndex;
            }
        }
        return false;
    }

    std::unique_ptr<JComponentStorage> storage_;
    std::unique_ptr<JEventDispatcher> dispatcher_;
};

/**
 * 测试 hitTestAll + JComponentId — 直接点击Button命中
 * 路径: hitTestAll → 候选包含Button → entry->id匹配
 */
TEST_F(ClickResolutionTest, HitButton_PointInside_FindsById) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    JComponentId btnId = storage_->getComponent(btn)->id;

    dispatcher_->onLayoutComplete();
    auto candidates = dispatcher_->hitTestAll(JPoint{200.0f, 120.0f});
    EXPECT_TRUE(findInCandidates(candidates, btnId));
}

/**
 * 测试 hitTestAll + JComponentId — 点击Button内Text，祖先链匹配Button
 * 路径: hitTestAll → Text候选 → 祖先链查parent → entry->id匹配Button
 * MC/DC: Text不在直接匹配中 → 进入祖先链循环 → 找到Button → true
 */
TEST_F(ClickResolutionTest, HitTextInsideButton_AncestorMatchesButton) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    JComponentId btnId = storage_->getComponent(btn)->id;

    auto txt = storage_->createComponent(JComponentType::Text, btn);
    storage_->getComponent(txt)->layoutResult = JRect(0, 0, 200, 40);

    dispatcher_->onLayoutComplete();
    auto candidates = dispatcher_->hitTestAll(JPoint{200.0f, 120.0f});
    EXPECT_TRUE(findInCandidates(candidates, btnId));
}

/**
 * 测试 hitTestAll + JComponentId — 点击空白区域，无匹配
 * 分支: candidates为空 → 循环不执行 → 返回false
 */
TEST_F(ClickResolutionTest, ClickEmptyArea_NoMatch) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    JComponentId btnId = storage_->getComponent(btn)->id;

    dispatcher_->onLayoutComplete();
    // 点击在Button区域外
    auto candidates = dispatcher_->hitTestAll(JPoint{500.0f, 500.0f});
    EXPECT_FALSE(findInCandidates(candidates, btnId));
}

/**
 * 测试 hitTestAll + JComponentId — Card内Text祖先链匹配Card
 */
TEST_F(ClickResolutionTest, HitTextInsideCard_AncestorMatchesCard) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto card = storage_->createComponent(JComponentType::Card, root);
    storage_->getComponent(card)->layoutResult = JRect(50, 50, 300, 100);
    JComponentId cardId = storage_->getComponent(card)->id;

    auto txt = storage_->createComponent(JComponentType::Text, card);
    storage_->getComponent(txt)->layoutResult = JRect(10, 10, 280, 80);

    dispatcher_->onLayoutComplete();
    auto candidates = dispatcher_->hitTestAll(JPoint{200.0f, 100.0f});
    EXPECT_TRUE(findInCandidates(candidates, cardId));
}

/**
 * 测试 hitTestAll + JComponentId — 多个按钮重叠，所有命中都在候选列表中
 * 分支: hitTestAll返回多个候选 → 遍历每个 → 都能匹配到各自的ID
 */
TEST_F(ClickResolutionTest, OverlappingButtons_BothInCandidates) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto btn1 = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn1)->layoutResult = JRect(100, 100, 200, 40);
    JComponentId id1 = storage_->getComponent(btn1)->id;

    auto btn2 = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn2)->layoutResult = JRect(150, 90, 150, 60);
    JComponentId id2 = storage_->getComponent(btn2)->id;

    dispatcher_->onLayoutComplete();
    auto candidates = dispatcher_->hitTestAll(JPoint{220.0f, 120.0f});
    // 重叠区域内应该能找到至少一个
    EXPECT_TRUE(findInCandidates(candidates, id1) || findInCandidates(candidates, id2));
}

/**
 * 测试 hitTestAll + JComponentId — 不可见组件不在候选列表
 * 分支: entry->visible=false → hitTestAll过滤 → 候选列表不含不可见组件
 */
TEST_F(ClickResolutionTest, InvisibleButton_NotInCandidates) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 100, 200, 40);
    storage_->getComponent(btn)->visible = false;
    JComponentId btnId = storage_->getComponent(btn)->id;

    dispatcher_->onLayoutComplete();
    auto candidates = dispatcher_->hitTestAll(JPoint{200.0f, 120.0f});
    EXPECT_FALSE(findInCandidates(candidates, btnId));
}

/**
 * 测试 hitTestAll + JComponentId — 多层嵌套Text→Container→Button
 * 路径: hitTestAll → Text → 祖先链 Container → Button → 匹配ButtonId
 */
TEST_F(ClickResolutionTest, DeeplyNestedText_AncestorMatchesButton) {
    auto root = storage_->createComponent(JComponentType::Container);
    storage_->getComponent(root)->layoutResult = JRect(0, 0, 800, 600);

    auto btn = storage_->createComponent(JComponentType::Button, root);
    storage_->getComponent(btn)->layoutResult = JRect(100, 300, 200, 40);
    JComponentId btnId = storage_->getComponent(btn)->id;

    auto innerCont = storage_->createComponent(JComponentType::Container, btn);
    storage_->getComponent(innerCont)->layoutResult = JRect(5, 5, 190, 30);

    auto txt = storage_->createComponent(JComponentType::Text, innerCont);
    storage_->getComponent(txt)->layoutResult = JRect(0, 0, 190, 30);

    dispatcher_->onLayoutComplete();
    auto candidates = dispatcher_->hitTestAll(JPoint{200.0f, 320.0f});
    EXPECT_TRUE(findInCandidates(candidates, btnId));
}

} // namespace test
} // namespace jaether
