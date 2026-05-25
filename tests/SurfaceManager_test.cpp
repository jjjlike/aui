// SurfaceManager_test.cpp
// JJSurfaceManager Surface生命周期管理 单元测试
//
// 测试覆盖标准: 语句覆盖、分支覆盖、条件覆盖、路径覆盖、MC/DC覆盖

#include "aether/SurfaceManager.h"
#include "aether/ComponentStorage.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

class SurfaceManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        storage_ = new JComponentStorage();
        sm_ = new JJSurfaceManager(*storage_);
    }
    void TearDown() override { delete sm_; delete storage_; }
    JComponentStorage* storage_;
    JJSurfaceManager* sm_;
};

// ---- createSurface ----

TEST_F(SurfaceManagerTest, CreateSurface_ValidId_ReturnsTrue) {
    EXPECT_TRUE(sm_->createSurface("main"));
    EXPECT_TRUE(sm_->hasSurface("main"));
}

TEST_F(SurfaceManagerTest, CreateSurface_DuplicateId_ReturnsFalse) {
    EXPECT_TRUE(sm_->createSurface("main"));
    EXPECT_FALSE(sm_->createSurface("main"));  // 重复创建
}

TEST_F(SurfaceManagerTest, CreateSurface_EmptyId_ReturnsFalse) {
    EXPECT_FALSE(sm_->createSurface(""));
    EXPECT_FALSE(sm_->hasSurface(""));
}

TEST_F(SurfaceManagerTest, CreateSurface_DefaultCatalogId_jaetherBasic) {
    sm_->createSurface("s1");
    EXPECT_EQ(sm_->getCatalogId("s1"), "jaether-basic");
}

TEST_F(SurfaceManagerTest, CreateSurface_CustomCatalogId_Stored) {
    sm_->createSurface("s1", "custom-catalog");
    EXPECT_EQ(sm_->getCatalogId("s1"), "custom-catalog");
}

TEST_F(SurfaceManagerTest, CreateSurface_MultipleSurfaces_AllExist) {
    EXPECT_TRUE(sm_->createSurface("s1"));
    EXPECT_TRUE(sm_->createSurface("s2"));
    EXPECT_TRUE(sm_->createSurface("s3"));
    EXPECT_TRUE(sm_->hasSurface("s1"));
    EXPECT_TRUE(sm_->hasSurface("s2"));
    EXPECT_TRUE(sm_->hasSurface("s3"));
}

// ---- deleteSurface ----

TEST_F(SurfaceManagerTest, DeleteSurface_Existing_Removes) {
    sm_->createSurface("main");
    sm_->deleteSurface("main");
    EXPECT_FALSE(sm_->hasSurface("main"));
}

TEST_F(SurfaceManagerTest, DeleteSurface_NonExisting_NoCrash) {
    EXPECT_NO_THROW(sm_->deleteSurface("nonexistent"));
}

// ---- registerComponent / findComponent ----

TEST_F(SurfaceManagerTest, RegisterAndFind_RoundTrip_Success) {
    sm_->createSurface("main");
    auto h = storage_->createComponent(JComponentType::Text);
    sm_->registerComponent("main", "title", h);
    
    JComponentHandle found = sm_->findComponent("main", "title");
    EXPECT_TRUE(found.isValid());
    EXPECT_EQ(found.index, h.index);
}

TEST_F(SurfaceManagerTest, RegisterComponent_ReRegister_Overwrites) {
    sm_->createSurface("main");
    auto h1 = storage_->createComponent(JComponentType::Text);
    auto h2 = storage_->createComponent(JComponentType::Button);
    sm_->registerComponent("main", "id1", h1);
    sm_->registerComponent("main", "id1", h2);  // 覆盖
    
    auto found = sm_->findComponent("main", "id1");
    EXPECT_EQ(found.index, h2.index);
}

TEST_F(SurfaceManagerTest, RegisterComponent_InvalidHandle_Ignored) {
    sm_->createSurface("main");
    JComponentHandle invalid;
    sm_->registerComponent("main", "id1", invalid);
    EXPECT_FALSE(sm_->findComponent("main", "id1").isValid());
}

TEST_F(SurfaceManagerTest, RegisterComponent_EmptyId_Ignored) {
    sm_->createSurface("main");
    auto h = storage_->createComponent(JComponentType::Text);
    sm_->registerComponent("main", "", h);
    EXPECT_TRUE(sm_->getAllComponentIds("main").empty());
}

TEST_F(SurfaceManagerTest, RegisterComponent_SurfaceNotExist_Ignored) {
    auto h = storage_->createComponent(JComponentType::Text);
    sm_->registerComponent("nonexistent", "id1", h);
    EXPECT_FALSE(sm_->findComponent("nonexistent", "id1").isValid());
}

TEST_F(SurfaceManagerTest, FindComponent_NonExisting_ReturnsInvalid) {
    sm_->createSurface("main");
    EXPECT_FALSE(sm_->findComponent("main", "nonexistent").isValid());
}

TEST_F(SurfaceManagerTest, FindComponent_SurfaceNotExist_ReturnsInvalid) {
    EXPECT_FALSE(sm_->findComponent("bad", "any").isValid());
}

// ---- findA2UIId ----

TEST_F(SurfaceManagerTest, FindA2UIId_Registered_ReturnsId) {
    sm_->createSurface("main");
    auto h = storage_->createComponent(JComponentType::Text);
    sm_->registerComponent("main", "myid", h);
    EXPECT_EQ(sm_->findA2UIId("main", h), "myid");
}

TEST_F(SurfaceManagerTest, FindA2UIId_NotRegistered_ReturnsEmpty) {
    sm_->createSurface("main");
    auto h = storage_->createComponent(JComponentType::Text);
    EXPECT_TRUE(sm_->findA2UIId("main", h).empty());
}

TEST_F(SurfaceManagerTest, FindA2UIId_SurfaceNotExist_ReturnsEmpty) {
    auto h = storage_->createComponent(JComponentType::Text);
    EXPECT_TRUE(sm_->findA2UIId("bad", h).empty());
}

TEST_F(SurfaceManagerTest, FindA2UIId_InvalidHandle_ReturnsEmpty) {
    sm_->createSurface("main");
    JComponentHandle invalid;
    EXPECT_TRUE(sm_->findA2UIId("main", invalid).empty());
}

// ---- unregisterComponent ----

TEST_F(SurfaceManagerTest, UnregisterComponent_RemovesMapping) {
    sm_->createSurface("main");
    auto h = storage_->createComponent(JComponentType::Text);
    sm_->registerComponent("main", "id1", h);
    sm_->unregisterComponent("main", "id1");
    EXPECT_FALSE(sm_->findComponent("main", "id1").isValid());
    EXPECT_EQ(sm_->getComponentCount("main"), 0);
}

TEST_F(SurfaceManagerTest, UnregisterComponent_NonExisting_NoCrash) {
    sm_->createSurface("main");
    EXPECT_NO_THROW(sm_->unregisterComponent("main", "nonexistent"));
    EXPECT_NO_THROW(sm_->unregisterComponent("bad", "any"));
}

// ---- getRootComponent / setRootComponent ----

TEST_F(SurfaceManagerTest, RootComponent_DefaultInvalid) {
    sm_->createSurface("main");
    EXPECT_FALSE(sm_->getRootComponent("main").isValid());
}

TEST_F(SurfaceManagerTest, RootComponent_SetAndGet_Success) {
    sm_->createSurface("main");
    auto h = storage_->createComponent(JComponentType::Container);
    sm_->setRootComponent("main", h);
    EXPECT_TRUE(sm_->getRootComponent("main").isValid());
    EXPECT_EQ(sm_->getRootComponent("main").index, h.index);
}

TEST_F(SurfaceManagerTest, RootComponent_SurfaceNotExist_ReturnsInvalid) {
    EXPECT_FALSE(sm_->getRootComponent("bad").isValid());
}

TEST_F(SurfaceManagerTest, SetRootComponent_SurfaceNotExist_Ignored) {
    auto h = storage_->createComponent(JComponentType::Container);
    EXPECT_NO_THROW(sm_->setRootComponent("bad", h));
}

// ---- getAllComponentIds / getAllComponentHandles / getComponentCount ----

TEST_F(SurfaceManagerTest, GetAllComponentIds_MultipleRegistrations_ReturnsAll) {
    sm_->createSurface("main");
    auto h1 = storage_->createComponent(JComponentType::Text);
    auto h2 = storage_->createComponent(JComponentType::Button);
    auto h3 = storage_->createComponent(JComponentType::Input);
    sm_->registerComponent("main", "a", h1);
    sm_->registerComponent("main", "b", h2);
    sm_->registerComponent("main", "c", h3);
    
    auto ids = sm_->getAllComponentIds("main");
    EXPECT_EQ(ids.size(), 3);
    EXPECT_EQ(sm_->getComponentCount("main"), 3);
    
    auto handles = sm_->getAllComponentHandles("main");
    EXPECT_EQ(handles.size(), 3);
}

TEST_F(SurfaceManagerTest, GetAllComponentIds_EmptySurface_ReturnsEmpty) {
    sm_->createSurface("main");
    EXPECT_TRUE(sm_->getAllComponentIds("main").empty());
    EXPECT_TRUE(sm_->getAllComponentHandles("main").empty());
}

TEST_F(SurfaceManagerTest, GetAllComponentIds_SurfaceNotExist_ReturnsEmpty) {
    EXPECT_TRUE(sm_->getAllComponentIds("bad").empty());
}

// ---- getAllSurfaceIds ----

TEST_F(SurfaceManagerTest, GetAllSurfaceIds_AfterCreation_ReturnsAll) {
    sm_->createSurface("s1");
    sm_->createSurface("s2");
    auto ids = sm_->getAllSurfaceIds();
    EXPECT_EQ(ids.size(), 2);
}

// ---- getCatalogId ----

TEST_F(SurfaceManagerTest, GetCatalogId_SurfaceNotExist_ReturnsEmpty) {
    EXPECT_TRUE(sm_->getCatalogId("bad").empty());
}

} // namespace test
} // namespace jaether
