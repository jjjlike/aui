// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// aether_app 样例应用程序
// 演示三区域布局：C++代码创建区 | JSON创建区 | 动态添加区
// JSON创建区和动态区域均通过外部JSON文件动态加载控件
// 所有UI创建代码都在此文件中，JAetherApplication仅提供框架能力

#include "aether/AetherApplication.h"
#include "aether/ControlRenderer.h"
#include "aether/A2UIParser.h"
#include "aether/SurfaceManager.h"
#include <windows.h>
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>

using namespace jaether;

// ==================== 共享工具函数 ====================

/**
 * 弹出文件选择对话框，选择JSON文件
 * @param hwnd 父窗口句柄（用于模态对话框）
 * @param outPath 输出参数，存储选择的文件路径
 * @return 用户选择了文件返回true，取消返回false
 */
static bool openJSONFileDialog(HWND hwnd, std::string& outPath) {
    wchar_t filename[MAX_PATH] = {0};

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"json";

    if (GetOpenFileNameW(&ofn)) {
        // 将宽字符路径转换为UTF-8
        int len = WideCharToMultiByte(CP_UTF8, 0, filename, -1, NULL, 0, NULL, NULL);
        outPath.resize(len);
        WideCharToMultiByte(CP_UTF8, 0, filename, -1, &outPath[0], len, NULL, NULL);
        // 去除结尾的\0
        while (!outPath.empty() && outPath.back() == '\0')
            outPath.pop_back();
        return true;
    }
    return false;
}

/**
 * 读取文本文件全部内容
 * @param filepath 文件路径
 * @param outContent 输出参数，存储文件内容
 * @return 读取成功返回true
 */
static bool readFileContent(const std::string& filepath, std::string& outContent) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) return false;

    file.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    outContent.resize(size);
    file.read(&outContent[0], static_cast<std::streamsize>(size));
    file.close();

    // 跳过UTF-8 BOM
    if (outContent.size() >= 3 &&
        static_cast<unsigned char>(outContent[0]) == 0xEF &&
        static_cast<unsigned char>(outContent[1]) == 0xBB &&
        static_cast<unsigned char>(outContent[2]) == 0xBF) {
        outContent.erase(0, 3);
    }

    return true;
}

/**
 * 验证JSON字符串是否包含有效的A2UI组件定义
 * @param json JSON字符串
 * @param outError 输出参数，错误描述
 * @return 有效返回true
 */
static bool validateA2UIJSON(const std::string& json, std::string& outError) {
    // 基础格式检查：必须包含 updateComponents 或 surfaceUpdate
    bool hasUpdateComponents = json.find("updateComponents") != std::string::npos;
    bool hasSurfaceUpdate = json.find("surfaceUpdate") != std::string::npos;

    if (!hasUpdateComponents && !hasSurfaceUpdate) {
        outError = "JSON文件缺少 updateComponents 或 surfaceUpdate 字段";
        return false;
    }

    // 检查是否包含components数组
    if (json.find("components") == std::string::npos) {
        outError = "JSON文件缺少 components 数组";
        return false;
    }

    // 检查是否包含有效的surfaceId
    if (json.find("surfaceId") == std::string::npos) {
        outError = "JSON文件缺少 surfaceId 字段";
        return false;
    }

    // 确认JSON整体格式可以被JJSONParser解析
    JJSONValue parsed = JJSONParser::parse(json);
    if (!std::holds_alternative<JJSONObject>(parsed.value)) {
        outError = "JSON根节点不是对象类型，无法解析";
        return false;
    }

    return true;
}

/**
 * 获取文件名（不含扩展名）作为surfaceId
 */
static std::string fileNameToSurfaceId(const std::string& filepath) {
    size_t slash = filepath.find_last_of("/\\");
    size_t dot = filepath.find_last_of('.');
    std::string fname;
    if (slash != std::string::npos) {
        fname = filepath.substr(slash + 1, dot - slash - 1);
    } else {
        fname = filepath.substr(0, dot);
    }
    // 移除非法字符，保留字母数字下划线
    fname.erase(std::remove_if(fname.begin(), fname.end(), [](char c) {
        return !std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-';
    }), fname.end());
    if (fname.empty()) fname = "imported";
    return "file-" + fname;
}

// ==================== 样例应用上下文 ====================

/**
 * 样例应用上下文结构体
 * 管理各区域控件状态、按钮句柄和点击事件处理
 */
struct SampleAppContext {
    // 按钮/容器标识（使用JComponentId替代JComponentHandle，避免index/generation匹配失效）
    JComponentId jsonLoadBtnId_ = INVALID_COMPONENT_ID;     // JSON创建区"加载JSON文件"按钮
    JComponentId dynamicToggleBtnId_ = INVALID_COMPONENT_ID; // 动态区域"动态添加/清空"按钮
    JComponentId jsonAreaContainerId_ = INVALID_COMPONENT_ID; // JSON创建区父容器
    JComponentId dynamicAreaContainerId_ = INVALID_COMPONENT_ID; // 动态区域父容器

    // 动态控件管理
    std::vector<JComponentHandle> jsonLoadedComponents_;   // JSON创建区加载的控件
    std::vector<JComponentHandle> dynamicComponents_;      // 动态区域加载的控件
    bool dynamicControlsShown_ = false;                     // 动态控件是否已显示

    // 事件队列
    JEvent pendingClickEvent_;                   // 待处理的点击事件（含坐标和目标）
    bool     hasPendingClick_ = false;              // 是否有待处理的点击（JEvent默认type=Click，不可靠）

    // 窗口句柄（用于文件对话框和消息弹窗）
    HWND hwnd_ = nullptr;

    /** 通过JComponentId获取当前的JComponentHandle，若组件已被销毁则返回无效句柄 */
    JComponentHandle resolveId(JLogicLayer& ll, JComponentId id) {
        if (id == INVALID_COMPONENT_ID) return JComponentHandle{};
        return ll.getStorage().findById(id);
    }

    /**
     * 在指定父容器中通过C++代码创建所有类型控件（静态样例）
     */
    std::vector<JComponentHandle> createAllControlsInArea(
        JLogicLayer& ll, JComponentHandle parent, const std::string& prefix) {
        std::vector<JComponentHandle> handles;
        float y = 30.0f, h = 28.0f, s = 8.0f, w = 360.0f;

        auto btn = ll.createComponent(JComponentType::Button, parent);
        ll.setProperty(btn, JPropertyId::X, JPropertyValue(0.0f));
        ll.setProperty(btn, JPropertyId::Y, JPropertyValue(y));
        ll.setProperty(btn, JPropertyId::Width, JPropertyValue(w));
        ll.setProperty(btn, JPropertyId::Height, JPropertyValue(h));
        ll.setProperty(btn, JPropertyId::Text, JPropertyValue(prefix + "按钮控件"));
        ll.setProperty(btn, JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));
        handles.push_back(btn); y += h + s;

        auto txt = ll.createComponent(JComponentType::Text, parent);
        ll.setProperty(txt, JPropertyId::X, JPropertyValue(0.0f));
        ll.setProperty(txt, JPropertyId::Y, JPropertyValue(y));
        ll.setProperty(txt, JPropertyId::Width, JPropertyValue(w));
        ll.setProperty(txt, JPropertyId::Height, JPropertyValue(h));
        ll.setProperty(txt, JPropertyId::Text, JPropertyValue(prefix + "文本标签"));
        ll.setProperty(txt, JPropertyId::FontSize, JPropertyValue(14.0f));
        handles.push_back(txt); y += h + s;

        auto inp = ll.createComponent(JComponentType::Input, parent);
        ll.setProperty(inp, JPropertyId::X, JPropertyValue(0.0f));
        ll.setProperty(inp, JPropertyId::Y, JPropertyValue(y));
        ll.setProperty(inp, JPropertyId::Width, JPropertyValue(w));
        ll.setProperty(inp, JPropertyId::Height, JPropertyValue(h));
        ll.setProperty(inp, JPropertyId::Text, JPropertyValue(prefix + "输入框"));
        handles.push_back(inp); y += h + s;

        auto card = ll.createComponent(JComponentType::Card, parent);
        ll.setProperty(card, JPropertyId::X, JPropertyValue(0.0f));
        ll.setProperty(card, JPropertyId::Y, JPropertyValue(y));
        ll.setProperty(card, JPropertyId::Width, JPropertyValue(w));
        ll.setProperty(card, JPropertyId::Height, JPropertyValue(36.0f));
        handles.push_back(card);
        auto cardTxt = ll.createComponent(JComponentType::Text, card);
        ll.setProperty(cardTxt, JPropertyId::X, JPropertyValue(10.0f));
        ll.setProperty(cardTxt, JPropertyId::Y, JPropertyValue(5.0f));
        ll.setProperty(cardTxt, JPropertyId::Width, JPropertyValue(w - 20.0f));
        ll.setProperty(cardTxt, JPropertyId::Height, JPropertyValue(26.0f));
        ll.setProperty(cardTxt, JPropertyId::Text, JPropertyValue(prefix + "卡片容器内含文本"));
        ll.setProperty(cardTxt, JPropertyId::FontSize, JPropertyValue(13.0f));
        handles.push_back(cardTxt); y += 36.0f + s;

        auto sub = ll.createComponent(JComponentType::Container, parent);
        ll.setProperty(sub, JPropertyId::X, JPropertyValue(0.0f));
        ll.setProperty(sub, JPropertyId::Y, JPropertyValue(y));
        ll.setProperty(sub, JPropertyId::Width, JPropertyValue(w));
        ll.setProperty(sub, JPropertyId::Height, JPropertyValue(h));
        ll.setProperty(sub, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Row)));
        handles.push_back(sub);
        auto s1 = ll.createComponent(JComponentType::Button, sub);
        ll.setProperty(s1, JPropertyId::X, JPropertyValue(0.0f));
        ll.setProperty(s1, JPropertyId::Y, JPropertyValue(0.0f));
        ll.setProperty(s1, JPropertyId::Width, JPropertyValue(170.0f));
        ll.setProperty(s1, JPropertyId::Height, JPropertyValue(h));
        ll.setProperty(s1, JPropertyId::Text, JPropertyValue(prefix + "子按钮1"));
        handles.push_back(s1);
        auto s2 = ll.createComponent(JComponentType::Button, sub);
        ll.setProperty(s2, JPropertyId::X, JPropertyValue(180.0f));
        ll.setProperty(s2, JPropertyId::Y, JPropertyValue(0.0f));
        ll.setProperty(s2, JPropertyId::Width, JPropertyValue(170.0f));
        ll.setProperty(s2, JPropertyId::Height, JPropertyValue(h));
        ll.setProperty(s2, JPropertyId::Text, JPropertyValue(prefix + "子按钮2"));
        handles.push_back(s2);

        return handles;
    }

    /**
     * 通过文件对话框加载JSON文件并创建控件（JSON创建区和动态区域共享）
     *
     * 完整流程:
     *   1. 打开文件选择对话框，让用户选择 .json 文件
     *   2. 读取文件内容
     *   3. 验证JSON格式是否为有效A2UI描述
     *   4. 调用JLogicLayer::loadFromA2UI解析并创建组件
     *   5. 将创建的组件挂到目标父容器下
     *   6. 为每个组件设置位置和尺寸
     *   7. 触发布局更新
     *   8. 如果已存在之前加载的控件则替换
     *
     * @param ll     逻辑层引用
     * @param parent 目标父容器句柄
     * @param existingComponents 之前加载的控件列表（将被销毁替换）
     * @param outNewComponents 输出参数，存储新创建的控件列表
     * @return 加载成功返回true
     */
    /** 将UTF-8字符串转换为宽字符，用于MessageBoxW等Win32 API */
    static std::wstring toWide(const std::string& utf8) {
        if (utf8.empty()) return L"";
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
        std::wstring result(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], len);
        while (!result.empty() && result.back() == L'\0') result.pop_back();
        return result;
    }

    /** 从A2UI JSON中提取surfaceId，JSON中无surfaceId时返回空字符串 */
    static std::string extractSurfaceIdFromJSON(const std::string& jsonContent) {
        auto parsed = JJSONParser::parse(jsonContent);
        if (!std::holds_alternative<JJSONObject>(parsed.value)) return "";
        const auto& rootObj = std::get<JJSONObject>(parsed.value);
        // 尝试 updateComponents.surfaceId
        auto ucIt = rootObj.find("updateComponents");
        if (ucIt != rootObj.end() && std::holds_alternative<JJSONObject>(ucIt->second.value)) {
            const auto& ucObj = std::get<JJSONObject>(ucIt->second.value);
            auto sidIt = ucObj.find("surfaceId");
            if (sidIt != ucObj.end() && std::holds_alternative<std::string>(sidIt->second.value)) {
                return std::get<std::string>(sidIt->second.value);
            }
        }
        // 尝试 surfaceUpdate.surfaceId（v0.8格式）
        auto suIt = rootObj.find("surfaceUpdate");
        if (suIt != rootObj.end() && std::holds_alternative<JJSONObject>(suIt->second.value)) {
            const auto& suObj = std::get<JJSONObject>(suIt->second.value);
            auto sidIt = suObj.find("surfaceId");
            if (sidIt != suObj.end() && std::holds_alternative<std::string>(sidIt->second.value)) {
                return std::get<std::string>(sidIt->second.value);
            }
        }
        return "";
    }

    bool loadControlsFromJSONFile(
        JLogicLayer& ll,
        JComponentHandle parent,
        std::vector<JComponentHandle>& existingComponents,
        std::vector<JComponentHandle>& outNewComponents) {

        // 步骤1：打开文件对话框
        std::string filepath;
        if (!openJSONFileDialog(hwnd_, filepath)) {
            return false;  // 用户取消选择
        }

        // 步骤2：读取文件内容
        std::string jsonContent;
        if (!readFileContent(filepath, jsonContent)) {
            MessageBoxW(hwnd_, toWide("无法读取文件:\n" + filepath).c_str(),
                        L"文件读取错误", MB_OK | MB_ICONERROR);
            return false;
        }

        if (jsonContent.empty()) {
            MessageBoxW(hwnd_, L"JSON 文件内容为空", L"格式错误", MB_OK | MB_ICONWARNING);
            return false;
        }

        // 步骤3：验证JSON格式
        std::string validationError;
        if (!validateA2UIJSON(jsonContent, validationError)) {
            MessageBoxW(hwnd_, toWide(validationError).c_str(), L"JSON验证失败", MB_OK | MB_ICONWARNING);
            return false;
        }

        // 步骤4：提取JSON中的实际surfaceId（parseAndApply内部使用此ID创建surface）
        std::string jsonSurfaceId = extractSurfaceIdFromJSON(jsonContent);
        if (jsonSurfaceId.empty()) {
            jsonSurfaceId = fileNameToSurfaceId(filepath);  // JSON中无surfaceId则用文件名生成
        }

        // 步骤5：销毁之前加载的控件（如果存在）
        for (auto& h : existingComponents) {
            if (h.isValid()) ll.destroyComponent(h);
        }
        existingComponents.clear();
        outNewComponents.clear();

        // 步骤6：使用A2UI解析器加载JSON并创建组件
        // 传入jsonSurfaceId确保parseAndApply使用与后续查找相同的surfaceId
        auto& parser = ll.getA2UIParser();
        std::string rootId = parser.parseAndApply(jsonContent, jsonSurfaceId);

        if (rootId.empty()) {
            MessageBoxW(hwnd_, L"JSON 解析失败，无法创建控件。\n请检查 JSON 格式是否正确。",
                        L"解析错误", MB_OK | MB_ICONERROR);
            return false;
        }

        // 步骤7：查找根组件（使用与parseAndApply相同的surfaceId）
        auto& sm = parser.getSurfaceManager();
        JComponentHandle jsonRoot = sm.findComponent(jsonSurfaceId, rootId);
        if (!jsonRoot.isValid()) {
            MessageBoxW(hwnd_, L"JSON 解析成功但未找到根组件", L"内部错误", MB_OK | MB_ICONERROR);
            return false;
        }

        // 建立父子关系
        auto* re = ll.getStorage().getComponent(jsonRoot);
        auto* pe = ll.getStorage().getComponent(parent);
        if (re && pe) {
            re->parentIndex = parent.index;
            pe->childrenIndices.push_back(jsonRoot.index);
        }

        // 步骤8：收集所有创建的控件句柄
        auto allIds = sm.getAllComponentIds(jsonSurfaceId);
        float baseY = 30.0f;

        for (const auto& id : allIds) {
            JComponentHandle hdl = sm.findComponent(jsonSurfaceId, id);
            if (!hdl.isValid()) continue;
            outNewComponents.push_back(hdl);

            auto* entry = ll.getStorage().getComponent(hdl);
            if (entry && id == rootId) {
                if (!entry->properties.hasProperty(JPropertyId::X)) {
                    ll.setProperty(hdl, JPropertyId::X, JPropertyValue(0.0f));
                }
                if (!entry->properties.hasProperty(JPropertyId::Y)) {
                    ll.setProperty(hdl, JPropertyId::Y, JPropertyValue(baseY));
                }
            }
        }

        // 步骤9：触发布局更新并重绘
        ll.runFrame();

        // 步骤10：反馈加载结果
        std::wstring successMsg = L"成功加载 " + std::to_wstring(outNewComponents.size()) + L" 个控件";
        SetWindowTextW(hwnd_, (std::wstring(L"jaether — ") + successMsg).c_str());

        return true;
    }

    /** 移除动态区域的所有控件并恢复按钮状态 */
    void removeDynamicControls(JLogicLayer& ll) {
        if (!dynamicControlsShown_) return;
        for (auto& h : dynamicComponents_) {
            if (h.isValid()) ll.destroyComponent(h);
        }
        dynamicComponents_.clear();
        // 通过JComponentId恢复按钮文字
        JComponentHandle toggleBtn = resolveId(ll, dynamicToggleBtnId_);
        if (toggleBtn.isValid()) {
            ll.setProperty(toggleBtn, JPropertyId::Text, JPropertyValue(std::string("动态添加")));
            ll.setProperty(toggleBtn, JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));
        }
        dynamicControlsShown_ = false;
        ll.runFrame();
    }

    /**
     * 处理点击事件：不信任dispatchMouseEvent传递的target，
     * 而是使用事件坐标自行执行hitTestAll，再通过JComponentId精确匹配
     *
     * 原因：dispatchMouseEvent的target可能因布局引擎/findInteractiveAncestor/
     * quadTree重建时序等原因返回错误组件。
     * 本方法完全绕过不可靠的target，自行查询点击位置的所有组件。
     */
    void handleClick(JLogicLayer& ll, const JEvent& clickEvent) {
        if (clickEvent.type != JEventType::Click) return;
        auto& storage = ll.getStorage();
        auto& dispatcher = ll.getEventDispatcher();

        // 步骤1：使用事件坐标自行命中测试，获取点击位置下的所有组件
        JPoint pt{clickEvent.mouse.position.x, clickEvent.mouse.position.y};
        auto candidates = dispatcher.hitTestAll(pt);

        // 步骤2：遍历所有候选组件及其祖先链，检查JComponentId是否匹配已知按钮
        for (const auto& h : candidates) {
            auto* entry = storage.getComponent(h);
            if (!entry) continue;

            // 直接匹配：组件的JComponentId是否等于已知按钮ID
            if (entry->id == jsonLoadBtnId_) {
                doJsonLoad(ll);
                return;
            }
            if (entry->id == dynamicToggleBtnId_) {
                doDynamicToggle(ll);
                return;
            }

            // 祖先链匹配：沿父链向上查找（处理点击Button内Text的情况）
            int32_t idx = h.index;
            for (int d = 0; d < 10; ++d) {
                auto* e = storage.getComponentByIndex(idx);
                if (!e || e->parentIndex < 0) break;
                auto* pe = storage.getComponentByIndex(e->parentIndex);
                if (!pe) break;
                if (pe->id == jsonLoadBtnId_) { doJsonLoad(ll); return; }
                if (pe->id == dynamicToggleBtnId_) { doDynamicToggle(ll); return; }
                idx = e->parentIndex;
            }
        }
    }

private:
    void doJsonLoad(JLogicLayer& ll) {
        std::vector<JComponentHandle> newComponents;
        JComponentHandle parent = resolveId(ll, jsonAreaContainerId_);
        if (parent.isValid()) {
            if (loadControlsFromJSONFile(ll, parent, jsonLoadedComponents_, newComponents)) {
                jsonLoadedComponents_ = std::move(newComponents);
            }
        }
    }

    void doDynamicToggle(JLogicLayer& ll) {
        if (dynamicControlsShown_) {
            removeDynamicControls(ll);
        } else {
            std::vector<JComponentHandle> newComponents;
            JComponentHandle parent = resolveId(ll, dynamicAreaContainerId_);
            if (parent.isValid()) {
                if (loadControlsFromJSONFile(ll, parent, dynamicComponents_, newComponents)) {
                    dynamicComponents_ = std::move(newComponents);
                    JComponentHandle toggleBtn = resolveId(ll, dynamicToggleBtnId_);
                    if (toggleBtn.isValid()) {
                        ll.setProperty(toggleBtn, JPropertyId::Text,
                                       JPropertyValue(std::string("清空")));
                    }
                    dynamicControlsShown_ = true;
                    ll.runFrame();
                }
            }
        }
    }

public:
};

// ==================== 创建完整样例UI ====================

void createSampleUI(SampleAppContext& ctx, JLogicLayer& ll) {
    // ===== 根容器 =====
    auto root = ll.createComponent(JComponentType::Container);
    ll.setProperty(root, JPropertyId::Width, JPropertyValue(1200.0f));
    ll.setProperty(root, JPropertyId::Height, JPropertyValue(700.0f));
    ll.setProperty(root, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Column)));

    // ===== 标题栏 =====
    auto titleBar = ll.createComponent(JComponentType::Container, root);
    ll.setProperty(titleBar, JPropertyId::Width, JPropertyValue(1200.0f));
    ll.setProperty(titleBar, JPropertyId::Height, JPropertyValue(40.0f));
    ll.setProperty(titleBar, JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));
    auto titleText = ll.createComponent(JComponentType::Text, titleBar);
    ll.setProperty(titleText, JPropertyId::X, JPropertyValue(10.0f));
    ll.setProperty(titleText, JPropertyId::Y, JPropertyValue(5.0f));
    ll.setProperty(titleText, JPropertyId::Width, JPropertyValue(1180.0f));
    ll.setProperty(titleText, JPropertyId::Height, JPropertyValue(30.0f));
    ll.setProperty(titleText, JPropertyId::Text, JPropertyValue(std::string("jaether 控件样例 — 左侧C++代码 | 中间JSON | 右侧动态")));
    ll.setProperty(titleText, JPropertyId::FontSize, JPropertyValue(16.0f));

    // ===== 三列主区域 =====
    auto mainRow = ll.createComponent(JComponentType::Container, root);
    ll.setProperty(mainRow, JPropertyId::Width, JPropertyValue(1200.0f));
    ll.setProperty(mainRow, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Row)));
    ll.setProperty(mainRow, JPropertyId::FlexGrow, JPropertyValue(1.0f));

    // ---- 左侧：C++代码创建区（静态样例）----
    auto leftCol = ll.createComponent(JComponentType::Container, mainRow);
    ll.setProperty(leftCol, JPropertyId::Width, JPropertyValue(380.0f));
    ll.setProperty(leftCol, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Column)));
    ll.setProperty(leftCol, JPropertyId::PaddingLeft, JPropertyValue(10.0f));
    ll.setProperty(leftCol, JPropertyId::PaddingTop, JPropertyValue(10.0f));
    auto leftTitle = ll.createComponent(JComponentType::Text, leftCol);
    ll.setProperty(leftTitle, JPropertyId::X, JPropertyValue(0.0f));
    ll.setProperty(leftTitle, JPropertyId::Y, JPropertyValue(0.0f));
    ll.setProperty(leftTitle, JPropertyId::Width, JPropertyValue(360.0f));
    ll.setProperty(leftTitle, JPropertyId::Height, JPropertyValue(24.0f));
    ll.setProperty(leftTitle, JPropertyId::Text, JPropertyValue(std::string("C++代码创建")));
    ll.setProperty(leftTitle, JPropertyId::FontSize, JPropertyValue(20.0f));
    ctx.createAllControlsInArea(ll, leftCol, "C++");

    // ---- 中间：JSON创建区（通过外部文件加载）----
    auto midCol = ll.createComponent(JComponentType::Container, mainRow);
    ll.setProperty(midCol, JPropertyId::Width, JPropertyValue(380.0f));
    ll.setProperty(midCol, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Column)));
    ll.setProperty(midCol, JPropertyId::PaddingLeft, JPropertyValue(10.0f));
    ll.setProperty(midCol, JPropertyId::PaddingTop, JPropertyValue(10.0f));
    ctx.jsonAreaContainerId_ = ll.getStorage().getComponent(midCol)->id;
    auto midTitle = ll.createComponent(JComponentType::Text, midCol);
    ll.setProperty(midTitle, JPropertyId::X, JPropertyValue(0.0f));
    ll.setProperty(midTitle, JPropertyId::Y, JPropertyValue(0.0f));
    ll.setProperty(midTitle, JPropertyId::Width, JPropertyValue(360.0f));
    ll.setProperty(midTitle, JPropertyId::Height, JPropertyValue(24.0f));
    ll.setProperty(midTitle, JPropertyId::Text, JPropertyValue(std::string("JSON加载")));
    ll.setProperty(midTitle, JPropertyId::FontSize, JPropertyValue(20.0f));
    // JSON加载按钮
    auto jsonLoadBtn = ll.createComponent(JComponentType::Button, midCol);
    ctx.jsonLoadBtnId_ = ll.getStorage().getComponent(jsonLoadBtn)->id;
    ll.setProperty(jsonLoadBtn, JPropertyId::X, JPropertyValue(0.0f));
    ll.setProperty(jsonLoadBtn, JPropertyId::Y, JPropertyValue(30.0f));
    ll.setProperty(jsonLoadBtn, JPropertyId::Width, JPropertyValue(160.0f));
    ll.setProperty(jsonLoadBtn, JPropertyId::Height, JPropertyValue(30.0f));
    ll.setProperty(jsonLoadBtn, JPropertyId::Text, JPropertyValue(std::string("加载JSON文件")));
    ll.setProperty(jsonLoadBtn, JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));

    // ---- 右侧：动态区域（运行时通过JSON文件加载）----
    auto rightCol = ll.createComponent(JComponentType::Container, mainRow);
    ll.setProperty(rightCol, JPropertyId::Width, JPropertyValue(380.0f));
    ll.setProperty(rightCol, JPropertyId::JFlexDirection, JPropertyValue(static_cast<int>(JFlexDirection::Column)));
    ll.setProperty(rightCol, JPropertyId::PaddingLeft, JPropertyValue(10.0f));
    ll.setProperty(rightCol, JPropertyId::PaddingTop, JPropertyValue(10.0f));
    ctx.dynamicAreaContainerId_ = ll.getStorage().getComponent(rightCol)->id;
    auto rightTitle = ll.createComponent(JComponentType::Text, rightCol);
    ll.setProperty(rightTitle, JPropertyId::X, JPropertyValue(0.0f));
    ll.setProperty(rightTitle, JPropertyId::Y, JPropertyValue(0.0f));
    ll.setProperty(rightTitle, JPropertyId::Width, JPropertyValue(360.0f));
    ll.setProperty(rightTitle, JPropertyId::Height, JPropertyValue(24.0f));
    ll.setProperty(rightTitle, JPropertyId::Text, JPropertyValue(std::string("动态区域")));
    ll.setProperty(rightTitle, JPropertyId::FontSize, JPropertyValue(20.0f));
    // 动态切换按钮
    auto dynamicToggleBtn = ll.createComponent(JComponentType::Button, rightCol);
    ctx.dynamicToggleBtnId_ = ll.getStorage().getComponent(dynamicToggleBtn)->id;
    ll.setProperty(dynamicToggleBtn, JPropertyId::X, JPropertyValue(0.0f));
    ll.setProperty(dynamicToggleBtn, JPropertyId::Y, JPropertyValue(30.0f));
    ll.setProperty(dynamicToggleBtn, JPropertyId::Width, JPropertyValue(120.0f));
    ll.setProperty(dynamicToggleBtn, JPropertyId::Height, JPropertyValue(30.0f));
    ll.setProperty(dynamicToggleBtn, JPropertyId::Text, JPropertyValue(std::string("动态添加")));
    ll.setProperty(dynamicToggleBtn, JPropertyId::BackgroundColor, JPropertyValue(std::string("primary")));

    ll.runFrame();
}

// ==================== 程序入口 ====================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    JAetherApplication app;

    if (!app.initialize(hInstance, nCmdShow)) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }

    // ---- 创建样例UI ----
    SampleAppContext ctx;
    ctx.hwnd_ = app.getHwnd();
    createSampleUI(ctx, *app.getLogicLayer());

    // ---- 注册鼠标点击回调（存储完整事件，包含坐标为handleClick使用）----
    app.getLogicLayer()->getEventDispatcher().setMouseCallback(
        [&ctx](JEvent& e) {
            if (e.type == JEventType::Click) {
                ctx.pendingClickEvent_ = e;
                ctx.hasPendingClick_ = true;
            }
        }
    );

    // ---- 注册每帧回调（使用事件坐标自行查找点击目标）----
    app.setFrameCallback([&ctx](JLogicLayer& ll) {
        if (ctx.hasPendingClick_) {
            ctx.hasPendingClick_ = false;
            ctx.handleClick(ll, ctx.pendingClickEvent_);
        }
    });

    // ---- 运行主循环 ----
    app.run();

    // 必须在CoUninitialize之前显式关闭
    app.shutdown();
    CoUninitialize();
    return 0;
}
