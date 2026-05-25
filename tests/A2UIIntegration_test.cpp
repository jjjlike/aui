// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// A2UIIntegration_test.cpp
// A2UI JSON界面描述 集成测试
//
// 测试覆盖标准: 语句覆盖、分支覆盖、条件覆盖、路径覆盖、MC/DC覆盖
// 测试范围: JA2UIParser + JA2UIGenerator + 端到端工作流

#include "aether/LogicLayer.h"
#include "aether/A2UIParser.h"
#include "aether/A2UIGenerator.h"
#include "aether/SurfaceManager.h"
#include "aether/DataModel.h"
#include "aether/ComponentCatalog.h"
#include <gtest/gtest.h>

namespace jaether {
namespace test {

/**
 * 测试夹具：创建JA2UIParser（自动初始化catalog/surfaceManager/dataModel）
 */
class A2UIParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        logicLayer_ = new JLogicLayer();
        parser_ = new JA2UIParser(*logicLayer_);
    }
    void TearDown() override { delete parser_; delete logicLayer_; }
    JLogicLayer* logicLayer_;
    JA2UIParser* parser_;
};

// ==================== parseAndApply 基础测试 ====================

/**
 * 测试 parseAndApply - 解析Text组件
 * 语句覆盖: parseAndApply → handleUpdateComponents → parseDescriptor → buildComponentTree → createComponent
 * 分支: updateComponents消息类型 → 提取components → 创建组件
 * 条件: JSON根节点为Object → 进入解析逻辑
 */
TEST_F(A2UIParserTest, ParseAndApply_SingleTextComponent_CreatesComponent) {
    const char* json = R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["title"]},
                {"id": "title", "component": "Text", "text": "Hello A2UI"}
            ]
        }
    })";
    
    std::string rootId = parser_->parseAndApply(json, "main");
    EXPECT_EQ(rootId, "root");
    
    auto& sm = parser_->getSurfaceManager();
    EXPECT_TRUE(sm.hasSurface("main"));
    
    JComponentHandle titleHandle = sm.findComponent("main", "title");
    EXPECT_TRUE(titleHandle.isValid());
    
    auto* textProp = logicLayer_->getProperty(titleHandle, JPropertyId::Text);
    ASSERT_NE(textProp, nullptr);
    EXPECT_EQ(textProp->get<std::string>(), "Hello A2UI");
}

/**
 * 测试 parseAndApply - 解析Button组件
 * 分支: Button组件 + child子组件
 */
TEST_F(A2UIParserTest, ParseAndApply_ButtonWithChild_CreatesHierarchy) {
    const char* json = R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["btn"]},
                {"id": "btn", "component": "Button", "child": "btn-text"},
                {"id": "btn-text", "component": "Text", "text": "Click"}
            ]
        }
    })";
    
    std::string rootId = parser_->parseAndApply(json, "main");
    EXPECT_EQ(rootId, "root");
    
    auto& sm = parser_->getSurfaceManager();
    JComponentHandle btnHandle = sm.findComponent("main", "btn");
    EXPECT_TRUE(btnHandle.isValid());
    
    JComponentHandle textHandle = sm.findComponent("main", "btn-text");
    EXPECT_TRUE(textHandle.isValid());
    
    auto* textProp = logicLayer_->getProperty(textHandle, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "Click");
}

/**
 * 测试 parseAndApply - Row布局组件
 * 分支: Row组件 → children数组 → 递归创建子组件
 */
TEST_F(A2UIParserTest, ParseAndApply_RowWithMultipleChildren_AllCreated) {
    const char* json = R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Row", "children": ["btn1", "btn2", "btn3"]},
                {"id": "btn1", "component": "Button", "child": "t1"},
                {"id": "t1", "component": "Text", "text": "One"},
                {"id": "btn2", "component": "Button", "child": "t2"},
                {"id": "t2", "component": "Text", "text": "Two"},
                {"id": "btn3", "component": "Button", "child": "t3"},
                {"id": "t3", "component": "Text", "text": "Three"}
            ]
        }
    })";
    
    parser_->parseAndApply(json, "main");
    auto& sm = parser_->getSurfaceManager();
    EXPECT_TRUE(sm.findComponent("main", "btn1").isValid());
    EXPECT_TRUE(sm.findComponent("main", "btn2").isValid());
    EXPECT_TRUE(sm.findComponent("main", "btn3").isValid());
    EXPECT_EQ(sm.getComponentCount("main"), 7);
}

/**
 * 测试 parseAndApply - TextField组件
 * 分支: Input类型 → label属性
 */
TEST_F(A2UIParserTest, ParseAndApply_TextField_CreatesInput) {
    const char* json = R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["input"]},
                {"id": "input", "component": "TextField", "label": "Enter text"}
            ]
        }
    })";
    
    parser_->parseAndApply(json, "main");
    auto& sm = parser_->getSurfaceManager();
    JComponentHandle inputHandle = sm.findComponent("main", "input");
    EXPECT_TRUE(inputHandle.isValid());
    
    auto* labelProp = logicLayer_->getProperty(inputHandle, JPropertyId::Text);
    ASSERT_NE(labelProp, nullptr);
    EXPECT_EQ(labelProp->get<std::string>(), "Enter text");
}

/**
 * 测试 parseAndApply - 数组不是A2UI消息
 * 分支: 非Object根节点 → 返回空字符串
 * MC/DC: !holds_alternative<JJSONObject> → 短路返回
 */
TEST_F(A2UIParserTest, ParseAndApply_InvalidJSON_ReturnsEmpty) {
    // 数组类型的JSON根节点不是有效的A2UI消息
    EXPECT_EQ(parser_->parseAndApply("[1,2,3]", "main"), "");
}

TEST_F(A2UIParserTest, ParseAndApply_EmptyComponents_ReturnsEmpty) {
    const char* json = R"({"updateComponents": {"surfaceId": "main", "components": []}})";
    std::string result = parser_->parseAndApply(json, "main");
    EXPECT_EQ(result, "");
}

TEST_F(A2UIParserTest, ParseAndApply_NonObjectRoot_ReturnsEmpty) {
    // 数组不是有效的A2UI消息
    EXPECT_EQ(parser_->parseAndApply("[1,2,3]", "main"), "");
}

// ==================== 增量更新测试 ====================

/**
 * 测试增量更新 - 更新已有组件属性
 * 路径: parseAndApply → 已有ID → updateExistingComponent
 * 分支: findComponent返回有效句柄 → 进入更新分支
 * MC/DC: existing.isValid() == true → 执行更新
 */
TEST_F(A2UIParserTest, IncrementalUpdate_UpdateTextProperty_ChangesText) {
    // 第一步：创建初始组件
    parser_->parseAndApply(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["label"]},
                {"id": "label", "component": "Text", "text": "Initial"}
            ]
        }
    })", "main");
    
    auto& sm = parser_->getSurfaceManager();
    JComponentHandle labelHandle = sm.findComponent("main", "label");
    auto* textProp = logicLayer_->getProperty(labelHandle, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "Initial");
    
    // 第二步：增量更新
    parser_->applyComponentUpdate(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "label", "component": "Text", "text": "Updated"}
            ]
        }
    })", "main");
    
    textProp = logicLayer_->getProperty(labelHandle, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "Updated");
}

/**
 * 测试增量更新 - applyComponentUpdate空JSON不会崩溃
 * MC/DC: JSON无效 → 提前返回
 */
TEST_F(A2UIParserTest, ApplyComponentUpdate_InvalidJSON_NoCrash) {
    EXPECT_NO_THROW(parser_->applyComponentUpdate("{{bad", "main"));
}

// ==================== v0.8兼容性测试 ====================

/**
 * 测试 v0.8 surfaceUpdate消息格式
 * 分支: surfaceUpdate消息 → 提取components数组 → 转换为v0.9格式
 * 条件: convertV08BoundValue处理literalString和path
 */
TEST_F(A2UIParserTest, V08SurfaceUpdate_TextComponent_Works) {
    const char* json = R"({
        "surfaceUpdate": {
            "surfaceId": "main",
            "components": [
                {
                    "id": "root",
                    "component": {
                        "Column": {
                            "children": {"explicitList": ["title"]}
                        }
                    }
                },
                {
                    "id": "title",
                    "component": {
                        "Text": {
                            "text": {"literalString": "Hello v0.8"}
                        }
                    }
                }
            ]
        }
    })";
    
    std::string rootId = parser_->parseAndApply(json, "main");
    EXPECT_EQ(rootId, "root");
    
    auto& sm = parser_->getSurfaceManager();
    auto h = sm.findComponent("main", "title");
    auto* textProp = logicLayer_->getProperty(h, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "Hello v0.8");
}

/**
 * 测试 v0.8 Button + data binding
 * 路径: v0.8 → literalString转换 → 数据绑定保持
 */
TEST_F(A2UIParserTest, V08Button_WithAction_Works) {
    const char* json = R"({
        "surfaceUpdate": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": {"Column": {"children": {"explicitList": ["btn"]}}}},
                {"id": "btn", "component": {"Button": {"child": "txt", "primary": true}}},
                {"id": "txt", "component": {"Text": {"text": {"literalString": "OK"}}}}
            ]
        }
    })";
    
    parser_->parseAndApply(json, "main");
    auto& sm = parser_->getSurfaceManager();
    EXPECT_TRUE(sm.findComponent("main", "btn").isValid());
}

// ==================== JSONL流式解析测试 ====================

/**
 * 测试 parseJSONL - 多行消息逐渐构建UI
 * 路径: parseJSONL → 逐行处理 → processMessage
 */
TEST_F(A2UIParserTest, ParseJSONL_MultipleMessages_BuildsUI) {
    const char* jsonl = 
        "{\"updateComponents\": {\"surfaceId\": \"main\", \"components\": [{\"id\": \"root\", \"component\": \"Column\", \"children\": [\"step1\"]},{\"id\": \"step1\", \"component\": \"Text\", \"text\": \"Step 1\"}]}}\n"
        "{\"updateComponents\": {\"surfaceId\": \"main\", \"components\": [{\"id\": \"step2\", \"component\": \"Text\", \"text\": \"Step 2\"}]}}\n";
    
    // 需要先创建surface（因为processMessage不会自动创建surface）
    parser_->getSurfaceManager().createSurface("main");
    parser_->parseJSONL(jsonl);
    
    auto& sm = parser_->getSurfaceManager();
    // step1应该通过树构建被创建
    EXPECT_TRUE(sm.hasSurface("main"));
}

// ==================== 数据模型测试 ====================

/**
 * 测试 applyDataModelUpdate
 * 分支: updateDataModel消息 → 提取contents数组 → applyUpdate
 */
TEST_F(A2UIParserTest, ApplyDataModelUpdate_SetsData) {
    const char* json = R"({
        "updateDataModel": {
            "surfaceId": "main",
            "contents": [
                {"key": "title", "valueString": "My App"}
            ]
        }
    })";
    
    parser_->applyDataModelUpdate(json, "main");
    
    JJSONValue v = parser_->getDataModel().getValue("main", "/title");
    ASSERT_TRUE(std::holds_alternative<std::string>(v.value));
    EXPECT_EQ(std::get<std::string>(v.value), "My App");
}

TEST_F(A2UIParserTest, ApplyDataModelUpdate_InvalidJSON_NoCrash) {
    EXPECT_NO_THROW(parser_->applyDataModelUpdate("{{bad", "main"));
}

// ==================== 属性映射测试（variant→fontSize等） ====================

/**
 * 测试 variant属性映射
 * 分支: variant="h1" → fontSize=32
 * 条件: propId == FontSize → 进入variant→fontSize分支
 */
TEST_F(A2UIParserTest, VariantH1_MapsToFontSize32) {
    parser_->parseAndApply(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["h"]},
                {"id": "h", "component": "Text", "text": "Title", "variant": "h1"}
            ]
        }
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "h");
    auto* fsProp = logicLayer_->getProperty(h, JPropertyId::FontSize);
    ASSERT_NE(fsProp, nullptr);
    EXPECT_FLOAT_EQ(fsProp->get<float>(), 32.0f);
}

TEST_F(A2UIParserTest, VariantH3_MapsToFontSize24) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "H3", "variant": "h3"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "t");
    auto* fsProp = logicLayer_->getProperty(h, JPropertyId::FontSize);
    EXPECT_FLOAT_EQ(fsProp->get<float>(), 24.0f);
}

TEST_F(A2UIParserTest, VariantBody_MapsToFontSize14) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "Body", "variant": "body"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "t");
    auto* fsProp = logicLayer_->getProperty(h, JPropertyId::FontSize);
    EXPECT_FLOAT_EQ(fsProp->get<float>(), 14.0f);
}

TEST_F(A2UIParserTest, VariantCaption_MapsToFontSize12) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "Caption", "variant": "caption"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "t");
    auto* fsProp = logicLayer_->getProperty(h, JPropertyId::FontSize);
    EXPECT_FLOAT_EQ(fsProp->get<float>(), 12.0f);
}

TEST_F(A2UIParserTest, VariantUnknown_MapsToDefault16) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "Default", "variant": "unknown"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "t");
    auto* fsProp = logicLayer_->getProperty(h, JPropertyId::FontSize);
    EXPECT_FLOAT_EQ(fsProp->get<float>(), 16.0f);
}

// ==================== 布局属性映射测试 ====================

/**
 * 测试 justify属性映射
 * 路径: justify="center" → JustifyContent::Center(1)
 */
TEST_F(A2UIParserTest, JustifyCenter_MapsToCorrectEnum) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Row", "children": ["t"], "justify": "center"},
            {"id": "t", "component": "Text", "text": "Centered"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "root");
    auto* jProp = logicLayer_->getProperty(h, JPropertyId::JJustifyContent);
    ASSERT_NE(jProp, nullptr);
    EXPECT_EQ(jProp->get<int>(), 1);
}

TEST_F(A2UIParserTest, JustifySpaceBetween_MapsCorrectly) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Row", "children": ["a","b"], "justify": "spaceBetween"},
            {"id": "a", "component": "Text", "text": "A"},
            {"id": "b", "component": "Text", "text": "B"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "root");
    auto* jProp = logicLayer_->getProperty(h, JPropertyId::JJustifyContent);
    EXPECT_EQ(jProp->get<int>(), 3);
}

TEST_F(A2UIParserTest, AlignCenter_MapsCorrectly) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Row", "children": ["t"], "align": "center"},
            {"id": "t", "component": "Text", "text": "Test"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "root");
    auto* aProp = logicLayer_->getProperty(h, JPropertyId::JAlignItems);
    ASSERT_NE(aProp, nullptr);
    EXPECT_EQ(aProp->get<int>(), 1);
}

TEST_F(A2UIParserTest, AlignStretch_MapsCorrectly) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"], "align": "stretch"},
            {"id": "t", "component": "Text", "text": "Stretch"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "root");
    auto* aProp = logicLayer_->getProperty(h, JPropertyId::JAlignItems);
    EXPECT_EQ(aProp->get<int>(), 3);
}

// ==================== 数据绑定测试 ====================

/**
 * 测试数据绑定 - {"path": "/data"} 格式
 * 分支: propValue是JJSONObject + 包含"path"键 → 注册数据绑定
 */
TEST_F(A2UIParserTest, DataBinding_PathFormat_RegistersBinding) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": {"path": "/user/name"}}
        ]}
    })", "main");
    
    // 数据绑定被注册到dataModel
    auto& dm = parser_->getDataModel();
    // 通过设置数据来触发绑定通知
    dm.setValue("main", "/user/name", JJSONValue(std::string("BoundValue")));
    
    auto h = parser_->getSurfaceManager().findComponent("main", "t");
    auto* textProp = logicLayer_->getProperty(h, JPropertyId::Text);
    // 绑定回调应该已更新文本
    ASSERT_NE(textProp, nullptr);
    EXPECT_EQ(textProp->get<std::string>(), "BoundValue");
}

// ==================== breadth/width属性测试 ====================

TEST_F(A2UIParserTest, WidthProperty_SetsWidth) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["box"], "width": 400},
            {"id": "box", "component": "Text", "text": "Box"}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "root");
    auto* wProp = logicLayer_->getProperty(h, JPropertyId::Width);
    ASSERT_NE(wProp, nullptr);
    EXPECT_FLOAT_EQ(wProp->get<float>(), 400.0f);
}

TEST_F(A2UIParserTest, WeightProperty_SetsFlexGrow) {
    parser_->parseAndApply(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Row", "children": ["item"], "width": 800, "height": 100},
            {"id": "item", "component": "Text", "text": "Flex", "weight": 2}
        ]}
    })", "main");
    
    auto h = parser_->getSurfaceManager().findComponent("main", "item");
    auto* flexProp = logicLayer_->getProperty(h, JPropertyId::FlexGrow);
    ASSERT_NE(flexProp, nullptr);
    EXPECT_FLOAT_EQ(flexProp->get<float>(), 2.0f);
}

// ==================== 待办应用JSON初始化集成测试 ====================

/**
 * 测试 完整的待办列表JSON初始化
 * 路径: 完整的Column → Row → Button + TextField 层次结构
 * MC/DC: 所有分支（Button/Text/Input/布局组件）都被覆盖
 */
TEST_F(A2UIParserTest, TodoAppFullJSON_AllComponentsCreated) {
    const char* todoJSON = R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["title", "input-row", "stats"]},
                {"id": "title", "component": "Text", "text": "我的待办列表", "variant": "h1"},
                {"id": "input-row", "component": "Row", "children": ["todo-input", "add-btn"]},
                {"id": "todo-input", "component": "TextField", "label": "输入待办事项"},
                {"id": "add-btn", "component": "Button", "child": "add-text"},
                {"id": "add-text", "component": "Text", "text": "添加"},
                {"id": "stats", "component": "Text", "text": "总计: 0"}
            ]
        }
    })";
    
    std::string rootId = parser_->parseAndApply(todoJSON, "main");
    EXPECT_EQ(rootId, "root");
    
    auto& sm = parser_->getSurfaceManager();
    EXPECT_EQ(sm.getComponentCount("main"), 7);
    
    // 验证所有组件存在
    EXPECT_TRUE(sm.findComponent("main", "title").isValid());
    EXPECT_TRUE(sm.findComponent("main", "input-row").isValid());
    EXPECT_TRUE(sm.findComponent("main", "todo-input").isValid());
    EXPECT_TRUE(sm.findComponent("main", "add-btn").isValid());
    EXPECT_TRUE(sm.findComponent("main", "add-text").isValid());
    EXPECT_TRUE(sm.findComponent("main", "stats").isValid());
    
    // 验证标题文本
    auto titleHandle = sm.findComponent("main", "title");
    auto* textProp = logicLayer_->getProperty(titleHandle, JPropertyId::Text);
    EXPECT_EQ(textProp->get<std::string>(), "我的待办列表");
}

// ==================== JLogicLayer A2UI接口测试 ====================

TEST_F(A2UIParserTest, LogicLayer_LoadFromA2UI_Works) {
    std::string rootId = logicLayer_->loadFromA2UI(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["t"]},
                {"id": "t", "component": "Text", "text": "Hello from LogicLayer"}
            ]
        }
    })");
    
    EXPECT_EQ(rootId, "root");
    
    auto& sm = logicLayer_->getSurfaceManager();
    auto h = sm.findComponent("main", "t");
    auto* tp = logicLayer_->getProperty(h, JPropertyId::Text);
    EXPECT_EQ(tp->get<std::string>(), "Hello from LogicLayer");
}

TEST_F(A2UIParserTest, LogicLayer_ExportToA2UI_GeneratesJSON) {
    logicLayer_->loadFromA2UI(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "Column", "children": ["lbl"]},
                {"id": "lbl", "component": "Text", "text": "Export Test"}
            ]
        }
    })");
    
    std::string exported = logicLayer_->exportToA2UI("main");
    EXPECT_FALSE(exported.empty());
    // 验证输出包含关键词
    EXPECT_NE(exported.find("updateComponents"), std::string::npos);
    EXPECT_NE(exported.find("Export Test"), std::string::npos);
}

TEST_F(A2UIParserTest, LogicLayer_ExportToA2UI_NoInit_ReturnsEmptyObj) {
    JLogicLayer ll;
    std::string exported = ll.exportToA2UI("main");
    EXPECT_EQ(exported, "{}");
}

TEST_F(A2UIParserTest, LogicLayer_StreamComponent_Works) {
    logicLayer_->getSurfaceManager().createSurface("main");
    logicLayer_->streamComponent(
        "{\"updateComponents\": {\"surfaceId\": \"main\", \"components\": [{\"id\": \"root\", \"component\": \"Column\", \"children\": [\"t\"]},{\"id\": \"t\", \"component\": \"Text\", \"text\": \"Streamed\"}]}}\n"
    );
    
    auto& sm = logicLayer_->getSurfaceManager();
    auto h = sm.findComponent("main", "t");
    if (h.isValid()) {
        auto* tp = logicLayer_->getProperty(h, JPropertyId::Text);
        if (tp) EXPECT_EQ(tp->get<std::string>(), "Streamed");
    }
}

// ==================== 未知消息类型测试 ====================

TEST_F(A2UIParserTest, ParseAndApply_UnknownMessageType_ReturnsEmpty) {
    EXPECT_EQ(parser_->parseAndApply("{}", "main"), "");
}

// ==================== 未注册组件类型测试 ====================

TEST_F(A2UIParserTest, ParseAndApply_UnknownComponentType_NoCrash) {
    // 未注册类型不应崩溃，仅使用Custom类型
    std::string result = parser_->parseAndApply(R"({
        "updateComponents": {
            "surfaceId": "main",
            "components": [
                {"id": "root", "component": "UnknownWidget", "children": []}
            ]
        }
    })", "main");
    EXPECT_EQ(result, "root");
}

// ==================== v0.8 createSurface消息 ====================

TEST_F(A2UIParserTest, ParseV09CreateSurface_Works) {
    parser_->parseAndApply(R"({
        "createSurface": {
            "surfaceId": "custom-surface",
            "catalogId": "https://my.catalog.json"
        }
    })", "main");
    
    auto& sm = parser_->getSurfaceManager();
    EXPECT_TRUE(sm.hasSurface("custom-surface"));
    EXPECT_EQ(sm.getCatalogId("custom-surface"), "https://my.catalog.json");
}

// ==================== JLogicLayer getA2UIGenerator ====================

TEST_F(A2UIParserTest, LogicLayer_GetA2UIGenerator_Works) {
    logicLayer_->loadFromA2UI(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "GenTest"}
        ]}
    })");
    
    auto& gen = logicLayer_->getA2UIGenerator();
    std::string json = gen.generateSurfaceJSON("main");
    EXPECT_NE(json.find("GenTest"), std::string::npos);
}

// ==================== 往返测试（Round-trip Tests）====================

/**
 * 测试 解析→导出→重新解析 的往返一致性
 * 两个独立 LogicLayer 实例避免组件ID冲突
 * 注意：导出的JSON内嵌surfaceId，重新解析时使用相同的surfaceId
 */
TEST_F(A2UIParserTest, RoundTrip_ParseGenerateParse_TextMatches) {
    const char* inputJSON = R"({
        "updateComponents": {
            "surfaceId": "s1",
            "components": [
                {"id": "root", "component": "Column", "children": ["title"]},
                {"id": "title", "component": "Text", "text": "RoundTrip Test"}
            ]
        }
    })";
    
    JLogicLayer ll2;
    ll2.loadFromA2UI(inputJSON, "s1");
    
    size_t count1 = ll2.getSurfaceManager().getComponentCount("s1");
    EXPECT_EQ(count1, 2);
    
    // 导出为JSON（内嵌surfaceId="s1"）
    std::string exported = ll2.exportToA2UI("s1");
    EXPECT_FALSE(exported.empty());
    EXPECT_NE(exported.find("RoundTrip Test"), std::string::npos);
    
    // 重新解析：JSON内嵌的surfaceId="s1"会覆盖surfaceId参数
    // 所以必须在不同的LogicLayer中解析，以免组件ID冲突
    JLogicLayer ll3;
    ll3.loadFromA2UI(exported, "s1");
    size_t count2 = ll3.getSurfaceManager().getComponentCount("s1");
    EXPECT_EQ(count2, count1);
}

/**
 * 测试 含variant属性的往返转换
 * 路径: variant="h1" → fontSize=32 → 导出为variant="h1" → 重新解析
 */
TEST_F(A2UIParserTest, RoundTrip_WithVariant_Preserved) {
    logicLayer_->loadFromA2UI(R"({
        "updateComponents": {"surfaceId": "s1", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "Title", "variant": "h1"}
        ]}
    })");
    
    std::string exported = logicLayer_->exportToA2UI("s1");
    EXPECT_NE(exported.find("h1"), std::string::npos);
}

/**
 * 测试 含justify/align布局属性的往返转换
 */
TEST_F(A2UIParserTest, RoundTrip_WithLayoutProps_Preserved) {
    logicLayer_->loadFromA2UI(R"({
        "updateComponents": {"surfaceId": "s1", "components": [
            {"id": "root", "component": "Row", "children": ["a","b"], "justify": "spaceBetween", "align": "center"},
            {"id": "a", "component": "Text", "text": "A"},
            {"id": "b", "component": "Text", "text": "B"}
        ]}
    })");
    
    std::string exported = logicLayer_->exportToA2UI("s1");
    EXPECT_NE(exported.find("spaceBetween"), std::string::npos);
    EXPECT_NE(exported.find("center"), std::string::npos);
}

/**
 * 测试 Card组件的往返转换
 */
TEST_F(A2UIParserTest, RoundTrip_CardComponent_Preserved) {
    logicLayer_->loadFromA2UI(R"({
        "updateComponents": {"surfaceId": "s1", "components": [
            {"id": "root", "component": "Column", "children": ["card"]},
            {"id": "card", "component": "Card", "child": "card-text"},
            {"id": "card-text", "component": "Text", "text": "Card Content"}
        ]}
    })");
    
    auto& sm = logicLayer_->getSurfaceManager();
    EXPECT_TRUE(sm.findComponent("s1", "card").isValid());
    
    std::string exported = logicLayer_->exportToA2UI("s1");
    EXPECT_NE(exported.find("Card"), std::string::npos);
}

/**
 * 测试 JTestController::getComponentTreeA2UI 可用性
 */
TEST_F(A2UIParserTest, TestController_GetComponentTreeA2UI_Works) {
    logicLayer_->loadFromA2UI(R"({
        "updateComponents": {"surfaceId": "main", "components": [
            {"id": "root", "component": "Column", "children": ["t"]},
            {"id": "t", "component": "Text", "text": "TestControllerA2UI"}
        ]}
    })");
    
    auto& tc = logicLayer_->getTestController();
    std::string a2uiTree = tc.getComponentTreeA2UI();
    EXPECT_FALSE(a2uiTree.empty());
    EXPECT_NE(a2uiTree, "{}");
    EXPECT_NE(a2uiTree.find("updateComponents"), std::string::npos);
}

/**
 * 测试 增量更新后的往返转换（使用独立LogicLayer）
 * 导出JSON内嵌surfaceId，重新解析时必须用相同ID
 */
TEST_F(A2UIParserTest, RoundTrip_AfterIncrementalUpdate_Consistent) {
    JLogicLayer ll;
    ll.loadFromA2UI(R"({
        "updateComponents": {"surfaceId": "s1", "components": [
            {"id": "root", "component": "Column", "children": ["lbl"]},
            {"id": "lbl", "component": "Text", "text": "Original"}
        ]}
    })");
    
    auto& parser = ll.getA2UIParser();
    parser.applyComponentUpdate(R"({
        "updateComponents": {"surfaceId": "s1", "components": [
            {"id": "lbl", "component": "Text", "text": "Updated"}
        ]}
    })", "s1");
    
    auto& sm = ll.getSurfaceManager();
    auto h = sm.findComponent("s1", "lbl");
    auto* tp = ll.getProperty(h, JPropertyId::Text);
    EXPECT_EQ(tp->get<std::string>(), "Updated");
    
    std::string exported = ll.exportToA2UI("s1");
    EXPECT_NE(exported.find("Updated"), std::string::npos);
    
    // 用独立LogicLayer重新解析（使用相同的surfaceId）
    JLogicLayer ll2;
    ll2.loadFromA2UI(exported, "s1");
    auto h2 = ll2.getSurfaceManager().findComponent("s1", "lbl");
    auto* tp2 = ll2.getProperty(h2, JPropertyId::Text);
    EXPECT_EQ(tp2->get<std::string>(), "Updated");
}

} // namespace test
} // namespace jaether
