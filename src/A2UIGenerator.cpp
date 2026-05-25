// A2UIGenerator.cpp
// A2UI JSON生成器模块 - 将jaether组件树导出为A2UI协议JSON描述
//
// 功能:
// - 将jaether组件树序列化为A2UI v0.9格式的JSON
// - 支持完整Surface导出和单组件导出
// - 处理类型逆向映射（枚举→字符串、字号→variant等）
// - 与JA2UIParser配对使用，确保双向一致性

#include "aether/A2UIGenerator.h"
#include "aether/Logger.h"
#include <sstream>

namespace jaether {

// 构造函数：初始化组件目录和引用
JA2UIGenerator::JA2UIGenerator(const JLogicLayer& logicLayer, 
                                 const JJSurfaceManager& surfaceManager)
    : logicLayer_(logicLayer)
    , surfaceManager_(surfaceManager)
    , catalog_(std::make_unique<JJComponentCatalog>()) {
}

// 生成完整Surface的A2UI JSON描述
// 输出v0.9 updateComponents消息格式
std::string JA2UIGenerator::generateSurfaceJSON(const std::string& surfaceId) const {
    if (!surfaceManager_.hasSurface(surfaceId)) {
        return "{}";
    }
    
    std::ostringstream oss;
    
    // 获取所有组件ID
    auto componentIds = surfaceManager_.getAllComponentIds(surfaceId);
    if (componentIds.empty()) {
        return "{}";
    }
    
    // 构建JSON
    oss << "{\n";
    oss << "  \"updateComponents\": {\n";
    oss << "    \"surfaceId\": \"" << surfaceId << "\",\n";
    oss << "    \"components\": [\n";
    
    bool first = true;
    for (const auto& a2uiId : componentIds) {
        JComponentHandle handle = surfaceManager_.findComponent(surfaceId, a2uiId);
        if (!handle.isValid()) continue;
        
        auto* entry = logicLayer_.getStorage().getComponent(handle);
        if (!entry) continue;
        
        if (!first) {
            oss << ",\n";
        }
        first = false;
        
        // 生成单个组件的JSON
        JJSONObject compJSON = generateComponentJSON(handle, a2uiId);
        oss << "      " << JJSONParser::stringify(JJSONValue(compJSON));
    }
    
    oss << "\n    ]\n";
    oss << "  }\n";
    oss << "}";
    
    return oss.str();
}

// 生成单个组件的A2UI JSON描述
JJSONObject JA2UIGenerator::generateComponentJSON(JComponentHandle handle, 
                                                    const std::string& a2uiId) const {
    JJSONObject comp;
    
    auto* entry = logicLayer_.getStorage().getComponent(handle);
    if (!entry) return comp;
    
    // 基本字段: id 和 component 类型名
    comp["id"] = JJSONValue(a2uiId);
    std::string a2uiType = catalog_->getA2UIName(entry->type);
    if (a2uiType.empty()) {
        // 对于未注册类型，使用Container作为兜底
        a2uiType = "Container";
    }
    comp["component"] = JJSONValue(a2uiType);
    
    // 导出children字段
    if (!entry->childrenIndices.empty()) {
        auto childrenIds = childrenToA2UIIDs(entry->childrenIndices,
            surfaceManager_.findA2UIId("", handle));  // 需要从surface查找
        
        // 实际上需要遍历所有surface来查找
        JJSONArray childrenArray;
        for (int32_t childIdx : entry->childrenIndices) {
            // 通过所有surface查找子组件的A2UI ID
            for (const auto& sid : surfaceManager_.getAllSurfaceIds()) {
                JComponentHandle childHandle{childIdx, 0};
                // 获取正确的generation
                auto* childEntry = logicLayer_.getStorage().getComponentByIndex(childIdx);
                if (childEntry) {
                    childHandle.generation = childEntry->generation;
                }
                std::string childA2UIId = surfaceManager_.findA2UIId(sid, childHandle);
                if (!childA2UIId.empty()) {
                    childrenArray.push_back(JJSONValue(childA2UIId));
                    break;
                }
            }
        }
        
        if (!childrenArray.empty()) {
            comp["children"] = JJSONValue(childrenArray);
        }
    }
    
    // 导出text属性
    auto* textProp = entry->properties.getProperty(JPropertyId::Text);
    if (textProp && textProp->is<std::string>()) {
        comp["text"] = JJSONValue(textProp->get<std::string>());
    }
    
    // 导出width和height属性
    if (entry->layoutResult.width > 0) {
        comp["width"] = JJSONValue(entry->layoutResult.width);
    }
    if (entry->layoutResult.height > 0) {
        comp["height"] = JJSONValue(entry->layoutResult.height);
    }
    
    // 导出flexGrow → weight
    auto* flexGrowProp = entry->properties.getProperty(JPropertyId::FlexGrow);
    if (flexGrowProp) {
        float fg = 0.0f;
        if (flexGrowProp->is<float>()) fg = flexGrowProp->get<float>();
        else if (flexGrowProp->is<int>()) fg = static_cast<float>(flexGrowProp->get<int>());
        if (fg > 0.0f) {
            comp["weight"] = JJSONValue(fg);
        }
    }
    
    // 导出fontSize → variant
    auto* fontSizeProp = entry->properties.getProperty(JPropertyId::FontSize);
    if (fontSizeProp) {
        float fs = 16.0f;
        if (fontSizeProp->is<float>()) fs = fontSizeProp->get<float>();
        else if (fontSizeProp->is<int>()) fs = static_cast<float>(fontSizeProp->get<int>());
        std::string variant = fontSizeToVariant(fs);
        if (!variant.empty()) {
            comp["variant"] = JJSONValue(variant);
        }
    }
    
    // 导出justifyContent → justify
    auto* justifyProp = entry->properties.getProperty(JPropertyId::JJustifyContent);
    if (justifyProp) {
        int jv = 0;
        if (justifyProp->is<int>()) jv = justifyProp->get<int>();
        std::string justifyStr = alignmentToString(jv, true);
        if (!justifyStr.empty()) {
            comp["justify"] = JJSONValue(justifyStr);
        }
    }
    
    // 导出alignItems → align
    auto* alignProp = entry->properties.getProperty(JPropertyId::JAlignItems);
    if (alignProp) {
        int av = 0;
        if (alignProp->is<int>()) av = alignProp->get<int>();
        std::string alignStr = alignmentToString(av, false);
        if (!alignStr.empty()) {
            comp["align"] = JJSONValue(alignStr);
        }
    }
    
    return comp;
}

// 生成数据模型的A2UI JSON
std::string JA2UIGenerator::generateDataModelJSON(const std::string& surfaceId) const {
    // 数据模型导出由JJDataModel负责
    // 此处生成包装的updateDataModel消息格式
    return "{}";
}

// 输出A2UI格式的组件树JSON
std::string JA2UIGenerator::getComponentTreeA2UI() const {
    // 遍历所有surface并导出
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"surfaces\": [\n";
    
    bool firstSurface = true;
    for (const auto& sid : surfaceManager_.getAllSurfaceIds()) {
        if (!firstSurface) oss << ",\n";
        firstSurface = false;
        oss << generateSurfaceJSON(sid);
    }
    
    oss << "\n  ]\n";
    oss << "}";
    return oss.str();
}

// 将jaether属性值转换为A2UI JSON值
JJSONValue JA2UIGenerator::propertyToA2UIValue(JPropertyId propId, 
                                                const JPropertyValue& value) const {
    if (propId == JPropertyId::Text) {
        if (value.is<std::string>()) {
            return JJSONValue(value.get<std::string>());
        }
    }
    // 其他属性的转换在 generateComponentJSON 中直接处理
    return JJSONValue();
}

// 将子组件索引列表转换为A2UI ID字符串列表
std::vector<std::string> JA2UIGenerator::childrenToA2UIIDs(
    const std::vector<int32_t>& childrenIndices, const std::string& surfaceId) const {
    std::vector<std::string> result;
    
    for (int32_t childIdx : childrenIndices) {
        auto* childEntry = logicLayer_.getStorage().getComponentByIndex(childIdx);
        if (!childEntry) continue;
        
        JComponentHandle childHandle{childIdx, childEntry->generation};
        
        // 在当前surface中查找A2UI ID
        std::string a2uiId = surfaceManager_.findA2UIId(surfaceId, childHandle);
        
        // 如果当前surface未找到，遍历所有surface
        if (a2uiId.empty()) {
            for (const auto& sid : surfaceManager_.getAllSurfaceIds()) {
                a2uiId = surfaceManager_.findA2UIId(sid, childHandle);
                if (!a2uiId.empty()) break;
            }
        }
        
        if (!a2uiId.empty()) {
            result.push_back(a2uiId);
        }
    }
    
    return result;
}

// 根据字号推理variant名称
std::string JA2UIGenerator::fontSizeToVariant(float fontSize) const {
    if (fontSize >= 32.0f) return "h1";
    if (fontSize >= 28.0f) return "h2";
    if (fontSize >= 24.0f) return "h3";
    if (fontSize >= 20.0f) return "h4";
    if (fontSize >= 16.0f) return "h5";
    if (fontSize >= 14.0f) return "body";
    if (fontSize > 0.0f)   return "caption";
    return "";
}

// 根据枚举值推理对齐字符串
std::string JA2UIGenerator::alignmentToString(int enumVal, bool isJustify) const {
    // JJustifyContent 和 JAlignItems 的枚举值
    // 0=FlexStart, 1=Center, 2=FlexEnd, 3=SpaceBetween, 4=SpaceAround
    switch (enumVal) {
        case 0: return "start";
        case 1: return "center";
        case 2: return "end";
        case 3: return isJustify ? "spaceBetween" : "stretch";
        case 4: return isJustify ? "spaceAround" : "";
        default: return "";
    }
}

} // namespace jaether
