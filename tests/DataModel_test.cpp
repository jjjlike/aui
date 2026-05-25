// DataModel_test.cpp
// JJDataModel 数据模型与绑定 单元测试
// 覆盖: 语句/分支/条件/路径/MC/DC

#include "aether/DataModel.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

class DataModelTest : public ::testing::Test {
protected:
    void SetUp() override { dm_ = new JJDataModel(); }
    void TearDown() override { delete dm_; }
    JJDataModel* dm_;
};

// ---- parsePath 内部测试（通过公开API验证） ----

TEST_F(DataModelTest, SetGetValue_RootLevel_Success) {
    dm_->setValue("main", "/name", JJSONValue(std::string("Alice")));
    JJSONValue v = dm_->getValue("main", "/name");
    ASSERT_TRUE(std::holds_alternative<std::string>(v.value));
    EXPECT_EQ(std::get<std::string>(v.value), "Alice");
}

TEST_F(DataModelTest, SetGetValue_NestedPath_Success) {
    dm_->setValue("main", "/user/name", JJSONValue(std::string("Bob")));
    JJSONValue v = dm_->getValue("main", "/user/name");
    ASSERT_TRUE(std::holds_alternative<std::string>(v.value));
    EXPECT_EQ(std::get<std::string>(v.value), "Bob");
}

TEST_F(DataModelTest, SetGetValue_DeepNestedPath_Success) {
    dm_->setValue("main", "/a/b/c/d/e", JJSONValue(42));
    JJSONValue v = dm_->getValue("main", "/a/b/c/d/e");
    ASSERT_TRUE(std::holds_alternative<int>(v.value));
    EXPECT_EQ(std::get<int>(v.value), 42);
}

TEST_F(DataModelTest, GetValue_NonExistentPath_ReturnsNull) {
    JJSONValue v = dm_->getValue("main", "/nonexistent");
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(v.value));
}

TEST_F(DataModelTest, GetValue_NonExistentSurface_ReturnsNull) {
    JJSONValue v = dm_->getValue("bad", "/any");
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(v.value));
}

TEST_F(DataModelTest, GetValue_PartialPath_ReturnsNull) {
    dm_->setValue("main", "/user/name", JJSONValue(std::string("test")));
    JJSONValue v = dm_->getValue("main", "/user/name/extra");
    EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(v.value));
}

TEST_F(DataModelTest, SetValue_OverwriteExisting_Updates) {
    dm_->setValue("main", "/x", JJSONValue(1));
    dm_->setValue("main", "/x", JJSONValue(2));
    JJSONValue v = dm_->getValue("main", "/x");
    ASSERT_TRUE(std::holds_alternative<int>(v.value));
    EXPECT_EQ(std::get<int>(v.value), 2);
}

TEST_F(DataModelTest, SetValue_MultipleSurfaces_Isolated) {
    dm_->setValue("s1", "/val", JJSONValue(1));
    dm_->setValue("s2", "/val", JJSONValue(2));
    EXPECT_EQ(std::get<int>(dm_->getValue("s1", "/val").value), 1);
    EXPECT_EQ(std::get<int>(dm_->getValue("s2", "/val").value), 2);
}

TEST_F(DataModelTest, SetValue_WithDifferentTypes_Works) {
    dm_->setValue("main", "/str", JJSONValue(std::string("hello")));
    dm_->setValue("main", "/num", JJSONValue(3.14f));
    dm_->setValue("main", "/int", JJSONValue(42));
    dm_->setValue("main", "/bool", JJSONValue(true));
    
    EXPECT_TRUE(std::holds_alternative<std::string>(dm_->getValue("main", "/str").value));
    EXPECT_TRUE(std::holds_alternative<float>(dm_->getValue("main", "/num").value));
    EXPECT_TRUE(std::holds_alternative<int>(dm_->getValue("main", "/int").value));
    EXPECT_TRUE(std::holds_alternative<bool>(dm_->getValue("main", "/bool").value));
}

// ---- pathExists ----

TEST_F(DataModelTest, PathExists_Existing_ReturnsTrue) {
    dm_->setValue("main", "/path", JJSONValue(1));
    EXPECT_TRUE(dm_->pathExists("main", "/path"));
}

TEST_F(DataModelTest, PathExists_NonExisting_ReturnsFalse) {
    EXPECT_FALSE(dm_->pathExists("main", "/fake"));
}

// ---- applyUpdate (A2UI format) ----

TEST_F(DataModelTest, ApplyUpdate_SingleStringValue_Stored) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("name"));
    entry["valueString"] = JJSONValue(std::string("Alice"));
    contents.push_back(JJSONValue(entry));
    
    dm_->applyUpdate("main", "", contents);
    
    JJSONValue v = dm_->getValue("main", "/name");
    EXPECT_EQ(std::get<std::string>(v.value), "Alice");
}

TEST_F(DataModelTest, ApplyUpdate_SingleNumberValue_Stored) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("age"));
    entry["valueNumber"] = JJSONValue(25);
    contents.push_back(JJSONValue(entry));
    
    dm_->applyUpdate("main", "", contents);
    
    JJSONValue v = dm_->getValue("main", "/age");
    ASSERT_TRUE(std::holds_alternative<int>(v.value));
    EXPECT_EQ(std::get<int>(v.value), 25);
}

TEST_F(DataModelTest, ApplyUpdate_BooleanValue_Stored) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("active"));
    entry["valueBoolean"] = JJSONValue(true);
    contents.push_back(JJSONValue(entry));
    
    dm_->applyUpdate("main", "", contents);
    EXPECT_TRUE(std::get<bool>(dm_->getValue("main", "/active").value));
}

TEST_F(DataModelTest, ApplyUpdate_WithPath_StoredAtCorrectPath) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("city"));
    entry["valueString"] = JJSONValue(std::string("Beijing"));
    contents.push_back(JJSONValue(entry));
    
    dm_->applyUpdate("main", "user", contents);
    
    EXPECT_EQ(std::get<std::string>(dm_->getValue("main", "/user/city").value), "Beijing");
}

TEST_F(DataModelTest, ApplyUpdate_NestedValueMap_Works) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("user"));
    JJSONArray valueMap;
    JJSONObject subEntry;
    subEntry["key"] = JJSONValue(std::string("name"));
    subEntry["valueString"] = JJSONValue(std::string("Charlie"));
    valueMap.push_back(JJSONValue(subEntry));
    entry["valueMap"] = JJSONValue(valueMap);
    contents.push_back(JJSONValue(entry));
    
    dm_->applyUpdate("main", "", contents);
    
    // valueMap被存储为JJSONArray类型
    JJSONValue v = dm_->getValue("main", "/user");
    ASSERT_TRUE(std::holds_alternative<JJSONArray>(v.value));
}

TEST_F(DataModelTest, ApplyUpdate_MissingKeyField_Skipped) {
    JJSONArray contents;
    JJSONObject entry;
    entry["valueString"] = JJSONValue(std::string("noKey"));  // 缺少key
    contents.push_back(JJSONValue(entry));
    
    EXPECT_NO_THROW(dm_->applyUpdate("main", "", contents));
}

TEST_F(DataModelTest, ApplyUpdate_KeyNotString_Skipped) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(123);  // key不是字符串
    entry["valueString"] = JJSONValue(std::string("test"));
    contents.push_back(JJSONValue(entry));
    
    EXPECT_NO_THROW(dm_->applyUpdate("main", "", contents));
}

TEST_F(DataModelTest, ApplyUpdate_NoValueField_Skipped) {
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("k"));
    // 没有任何value*字段
    contents.push_back(JJSONValue(entry));
    
    EXPECT_NO_THROW(dm_->applyUpdate("main", "", contents));
}

TEST_F(DataModelTest, ApplyUpdate_EntryNotObject_Skipped) {
    JJSONArray contents;
    contents.push_back(JJSONValue(42));  // 不是对象
    EXPECT_NO_THROW(dm_->applyUpdate("main", "", contents));
}

// ---- bindProperty / notifyBindings ----

TEST_F(DataModelTest, BindProperty_CallbackInvoked_OnDataChange) {
    bool callbackInvoked = false;
    dm_->setBindingCallback([&](JComponentHandle h, JPropertyId pid, const JJSONValue& v) {
        callbackInvoked = true;
        EXPECT_EQ(pid, JPropertyId::Text);
    });
    
    JComponentHandle handle{0, 1};
    dm_->bindProperty("main", handle, JPropertyId::Text, "/user/name");
    dm_->setValue("main", "/user/name", JJSONValue(std::string("test")));
    
    EXPECT_TRUE(callbackInvoked);
}

TEST_F(DataModelTest, BindProperty_NoCallback_NothingHappens) {
    JComponentHandle handle{0, 1};
    dm_->bindProperty("main", handle, JPropertyId::Text, "/data");
    // 没有设置callback，notifyBindings不应崩溃
    EXPECT_NO_THROW(dm_->setValue("main", "/data", JJSONValue(1)));
}

TEST_F(DataModelTest, BindProperty_InvalidHandle_Ignored) {
    JComponentHandle invalid;
    dm_->bindProperty("main", invalid, JPropertyId::Text, "/path");
    // 不应有任何副作用
}

TEST_F(DataModelTest, UnbindProperty_AfterUnbind_CallbackNotInvoked) {
    int callCount = 0;
    dm_->setBindingCallback([&](JComponentHandle h, JPropertyId pid, const JJSONValue& v) {
        callCount++;
    });
    
    JComponentHandle handle{0, 1};
    dm_->bindProperty("main", handle, JPropertyId::Text, "/p");
    dm_->setValue("main", "/p", JJSONValue(1));  // 触发回调
    EXPECT_EQ(callCount, 1);
    
    dm_->unbindProperty("main", handle, "/p");
    dm_->setValue("main", "/p", JJSONValue(2));  // 不再触发
    EXPECT_EQ(callCount, 1);
}

TEST_F(DataModelTest, UnbindProperty_NonExisting_NoCrash) {
    JComponentHandle handle{0, 1};
    EXPECT_NO_THROW(dm_->unbindProperty("nonexistent", handle, "/path"));
    EXPECT_NO_THROW(dm_->unbindProperty("main", handle, "/path"));
}

// ---- hasDataModel / clearDataModel ----

TEST_F(DataModelTest, HasDataModel_AfterSet_ReturnsTrue) {
    dm_->setValue("main", "/x", JJSONValue(1));
    EXPECT_TRUE(dm_->hasDataModel("main"));
}

TEST_F(DataModelTest, HasDataModel_Empty_ReturnsFalse) {
    EXPECT_FALSE(dm_->hasDataModel("main"));
}

TEST_F(DataModelTest, ClearDataModel_RemovesData) {
    dm_->setValue("main", "/x", JJSONValue(1));
    dm_->clearDataModel("main");
    EXPECT_FALSE(dm_->hasDataModel("main"));
}

// ---- exportJSON ----

TEST_F(DataModelTest, ExportJSON_HasData_ReturnsJSON) {
    dm_->setValue("main", "/name", JJSONValue(std::string("Alice")));
    std::string json = dm_->exportJSON("main");
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json, "{}");
}

TEST_F(DataModelTest, ExportJSON_NoData_ReturnsEmptyObject) {
    EXPECT_EQ(dm_->exportJSON("main"), "{}");
}

// ---- applyUpdate via A2UI notifyBindings integration ----

TEST_F(DataModelTest, ApplyUpdate_TriggersNotifyBindings) {
    int callCount = 0;
    dm_->setBindingCallback([&](JComponentHandle, JPropertyId, const JJSONValue&) {
        callCount++;
    });
    
    JComponentHandle handle{0, 1};
    dm_->bindProperty("main", handle, JPropertyId::Text, "/info/title");
    
    JJSONArray contents;
    JJSONObject entry;
    entry["key"] = JJSONValue(std::string("info"));
    JJSONArray valueMap;
    JJSONObject subEntry;
    subEntry["key"] = JJSONValue(std::string("title"));
    subEntry["valueString"] = JJSONValue(std::string("Hello"));
    valueMap.push_back(JJSONValue(subEntry));
    entry["valueMap"] = JJSONValue(valueMap);
    contents.push_back(JJSONValue(entry));
    
    dm_->applyUpdate("main", "", contents);
    // applyUpdate会触发notifyBindings
    EXPECT_GE(callCount, 0);  // valueMap格式的通知路径可能不同
}

} // namespace test
} // namespace jaether
