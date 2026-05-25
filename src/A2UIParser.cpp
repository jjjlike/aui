// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// A2UIParser.cpp
// A2UI JSON解析器模块 - 将A2UI协议JSON描述解析为jaether组件树

#include "aether/A2UIParser.h"
#include "aether/Logger.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <functional>

namespace jaether {

JA2UIParser::JA2UIParser(JLogicLayer& logicLayer)
    : logicLayer_(logicLayer)
    , catalog_(std::make_unique<JJComponentCatalog>())
    , surfaceManager_(std::make_unique<JJSurfaceManager>(logicLayer_.getStorage()))
    , dataModel_(std::make_unique<JJDataModel>()) {
    
    dataModel_->setBindingCallback(
        [this](JComponentHandle handle, JPropertyId propId, const JJSONValue& value) {
            if (std::holds_alternative<std::string>(value.value)) {
                logicLayer_.setProperty(handle, propId, 
                    JPropertyValue(std::get<std::string>(value.value)));
            } else if (std::holds_alternative<int>(value.value)) {
                logicLayer_.setProperty(handle, propId, 
                    JPropertyValue(std::get<int>(value.value)));
            } else if (std::holds_alternative<float>(value.value)) {
                logicLayer_.setProperty(handle, propId, 
                    JPropertyValue(std::get<float>(value.value)));
            } else if (std::holds_alternative<bool>(value.value)) {
                logicLayer_.setProperty(handle, propId, 
                    JPropertyValue(std::get<bool>(value.value)));
            }
        }
    );
}

std::string JA2UIParser::parseAndApply(const std::string& json, const std::string& surfaceId) {
    JJSONValue root = JJSONParser::parse(json);
    if (!std::holds_alternative<JJSONObject>(root.value)) {
        return "";
    }
    const auto& rootObj = std::get<JJSONObject>(root.value);
    
    auto updateIt = rootObj.find("updateComponents");
    if (updateIt != rootObj.end() && std::holds_alternative<JJSONObject>(updateIt->second.value)) {
        return handleUpdateComponents(std::get<JJSONObject>(updateIt->second.value));
    }
    
    auto surfaceUpdateIt = rootObj.find("surfaceUpdate");
    if (surfaceUpdateIt != rootObj.end() && std::holds_alternative<JJSONObject>(surfaceUpdateIt->second.value)) {
        const auto& updateObj = std::get<JJSONObject>(surfaceUpdateIt->second.value);
        std::string sid = surfaceId;
        auto sidIt = updateObj.find("surfaceId");
        if (sidIt != updateObj.end() && std::holds_alternative<std::string>(sidIt->second.value)) {
            sid = std::get<std::string>(sidIt->second.value);
        }
        if (!surfaceManager_->hasSurface(sid)) surfaceManager_->createSurface(sid);
        auto compsIt = updateObj.find("components");
        if (compsIt != updateObj.end() && std::holds_alternative<JJSONArray>(compsIt->second.value)) {
            const auto& compsArray = std::get<JJSONArray>(compsIt->second.value);
            std::vector<ComponentDescriptor> descriptors;
            for (const auto& comp : compsArray) {
                if (std::holds_alternative<JJSONObject>(comp.value)) {
                    descriptors.push_back(parseV08Component(std::get<JJSONObject>(comp.value)));
                }
            }
            return buildComponentTree(descriptors, sid);
        }
    }
    
    auto dataUpdateIt = rootObj.find("updateDataModel");
    if (dataUpdateIt != rootObj.end() && std::holds_alternative<JJSONObject>(dataUpdateIt->second.value)) {
        handleUpdateDataModel(std::get<JJSONObject>(dataUpdateIt->second.value));
        return "";
    }
    
    auto dataModelUpdateIt = rootObj.find("dataModelUpdate");
    if (dataModelUpdateIt != rootObj.end() && std::holds_alternative<JJSONObject>(dataModelUpdateIt->second.value)) {
        handleUpdateDataModel(std::get<JJSONObject>(dataModelUpdateIt->second.value));
        return "";
    }
    
    auto createSurfaceIt = rootObj.find("createSurface");
    if (createSurfaceIt != rootObj.end() && std::holds_alternative<JJSONObject>(createSurfaceIt->second.value)) {
        handleCreateSurface(std::get<JJSONObject>(createSurfaceIt->second.value));
        return "";
    }
    
    return "";
}

std::string JA2UIParser::handleUpdateComponents(const JJSONObject& updateObj) {
    std::string surfaceId;
    auto sidIt = updateObj.find("surfaceId");
    if (sidIt != updateObj.end() && std::holds_alternative<std::string>(sidIt->second.value)) {
        surfaceId = std::get<std::string>(sidIt->second.value);
    }
    if (surfaceId.empty()) return "";
    
    if (!surfaceManager_->hasSurface(surfaceId)) surfaceManager_->createSurface(surfaceId);
    
    auto compsIt = updateObj.find("components");
    if (compsIt == updateObj.end() || !std::holds_alternative<JJSONArray>(compsIt->second.value)) return "";
    
    const auto& compsArray = std::get<JJSONArray>(compsIt->second.value);
    std::vector<ComponentDescriptor> descriptors;
    for (const auto& comp : compsArray) {
        if (std::holds_alternative<JJSONObject>(comp.value)) {
            descriptors.push_back(parseDescriptor(std::get<JJSONObject>(comp.value)));
        }
    }
    return buildComponentTree(descriptors, surfaceId);
}

void JA2UIParser::handleCreateSurface(const JJSONObject& createObj) {
    std::string surfaceId;
    auto sidIt = createObj.find("surfaceId");
    if (sidIt != createObj.end() && std::holds_alternative<std::string>(sidIt->second.value))
        surfaceId = std::get<std::string>(sidIt->second.value);
    std::string catalogId = "jaether-basic";
    auto cidIt = createObj.find("catalogId");
    if (cidIt != createObj.end() && std::holds_alternative<std::string>(cidIt->second.value))
        catalogId = std::get<std::string>(cidIt->second.value);
    surfaceManager_->createSurface(surfaceId, catalogId);
}

void JA2UIParser::handleUpdateDataModel(const JJSONObject& dataObj) {
    std::string surfaceId;
    auto sidIt = dataObj.find("surfaceId");
    if (sidIt != dataObj.end() && std::holds_alternative<std::string>(sidIt->second.value))
        surfaceId = std::get<std::string>(sidIt->second.value);
    std::string path;
    auto pathIt = dataObj.find("path");
    if (pathIt != dataObj.end() && std::holds_alternative<std::string>(pathIt->second.value))
        path = std::get<std::string>(pathIt->second.value);
    auto contentsIt = dataObj.find("contents");
    if (contentsIt != dataObj.end() && std::holds_alternative<JJSONArray>(contentsIt->second.value))
        dataModel_->applyUpdate(surfaceId, path, std::get<JJSONArray>(contentsIt->second.value));
}

JA2UIParser::ComponentDescriptor JA2UIParser::parseDescriptor(const JJSONObject& obj) {
    ComponentDescriptor desc;
    auto idIt = obj.find("id");
    if (idIt != obj.end() && std::holds_alternative<std::string>(idIt->second.value))
        desc.id = std::get<std::string>(idIt->second.value);
    auto compIt = obj.find("component");
    if (compIt != obj.end() && std::holds_alternative<std::string>(compIt->second.value))
        desc.type = std::get<std::string>(compIt->second.value);
    
    auto childrenIt = obj.find("children");
    if (childrenIt != obj.end() && std::holds_alternative<JJSONArray>(childrenIt->second.value)) {
        const auto& arr = std::get<JJSONArray>(childrenIt->second.value);
        for (const auto& c : arr) {
            if (std::holds_alternative<std::string>(c.value))
                desc.children.push_back(std::get<std::string>(c.value));
        }
    }
    
    auto childIt = obj.find("child");
    if (childIt != obj.end() && std::holds_alternative<std::string>(childIt->second.value))
        desc.child = std::get<std::string>(childIt->second.value);
    
    for (const auto& kv : obj) {
        if (kv.first != "id" && kv.first != "component" && 
            kv.first != "children" && kv.first != "child") {
            desc.properties[kv.first] = kv.second;
            if (std::holds_alternative<JJSONObject>(kv.second.value)) {
                const auto& inner = std::get<JJSONObject>(kv.second.value);
                if (inner.find("path") != inner.end()) desc.hasDataBinding = true;
            }
        }
    }
    return desc;
}

JA2UIParser::ComponentDescriptor JA2UIParser::parseV08Component(const JJSONObject& obj) {
    ComponentDescriptor desc;
    auto idIt = obj.find("id");
    if (idIt != obj.end() && std::holds_alternative<std::string>(idIt->second.value))
        desc.id = std::get<std::string>(idIt->second.value);
    
    auto compIt = obj.find("component");
    if (compIt != obj.end() && std::holds_alternative<JJSONObject>(compIt->second.value)) {
        const auto& cw = std::get<JJSONObject>(compIt->second.value);
        if (!cw.empty()) {
            desc.type = cw.begin()->first;
            if (std::holds_alternative<JJSONObject>(cw.begin()->second.value)) {
                const auto& props = std::get<JJSONObject>(cw.begin()->second.value);
                for (const auto& kv : props) {
                    if (kv.first == "children") {
                        if (std::holds_alternative<JJSONObject>(kv.second.value)) {
                            const auto& cw2 = std::get<JJSONObject>(kv.second.value);
                            auto el = cw2.find("explicitList");
                            if (el != cw2.end() && std::holds_alternative<JJSONArray>(el->second.value)) {
                                for (const auto& c : std::get<JJSONArray>(el->second.value))
                                    if (std::holds_alternative<std::string>(c.value))
                                        desc.children.push_back(std::get<std::string>(c.value));
                            }
                        }
                    } else if (kv.first == "child") {
                        if (std::holds_alternative<std::string>(kv.second.value))
                            desc.child = std::get<std::string>(kv.second.value);
                    } else if (kv.first == "usageHint") {
                        desc.properties["variant"] = convertV08BoundValue(kv.second);
                    } else if (kv.first == "distribution") {
                        desc.properties["justify"] = convertV08BoundValue(kv.second);
                    } else if (kv.first == "alignment") {
                        desc.properties["align"] = convertV08BoundValue(kv.second);
                    } else if (kv.first == "primary") {
                        desc.properties["primary"] = kv.second;
                    } else {
                        JJSONValue cv = convertV08BoundValue(kv.second);
                        desc.properties[kv.first] = cv;
                        if (std::holds_alternative<JJSONObject>(cv.value)) {
                            if (std::get<JJSONObject>(cv.value).find("path") != std::get<JJSONObject>(cv.value).end())
                                desc.hasDataBinding = true;
                        }
                    }
                }
            }
        }
    }
    return desc;
}

JJSONValue JA2UIParser::convertV08BoundValue(const JJSONValue& value) {
    if (!std::holds_alternative<JJSONObject>(value.value)) return value;
    const auto& obj = std::get<JJSONObject>(value.value);
    auto lit = obj.find("literalString");
    if (lit != obj.end()) return lit->second;
    auto num = obj.find("literalNumber");
    if (num != obj.end()) return num->second;
    auto b = obj.find("literalBoolean");
    if (b != obj.end()) return b->second;
    return value;
}

JComponentHandle JA2UIParser::createComponent(const ComponentDescriptor& desc, JComponentHandle parent) {
    JComponentType t = catalog_->getType(desc.type);
    return logicLayer_.createComponent(t, parent);
}

void JA2UIParser::updateExistingComponent(JComponentHandle handle, const ComponentDescriptor& desc) {
    for (const auto& sid : surfaceManager_->getAllSurfaceIds()) {
        if (surfaceManager_->findA2UIId(sid, handle) == desc.id) {
            for (const auto& kv : desc.properties)
                applyProperty(handle, kv.first, kv.second, sid);
            break;
        }
    }
}

std::string JA2UIParser::buildComponentTree(const std::vector<ComponentDescriptor>& descriptors,
                                             const std::string& surfaceId) {
    if (descriptors.empty()) return "";
    
    std::unordered_map<std::string, const ComponentDescriptor*> descMap;
    for (const auto& desc : descriptors) descMap[desc.id] = &desc;
    
    std::unordered_set<std::string> referencedIds;
    for (const auto& desc : descriptors) {
        for (const auto& c : desc.children) referencedIds.insert(c);
        if (!desc.child.empty()) referencedIds.insert(desc.child);
    }
    
    std::string rootId;
    for (const auto& desc : descriptors) {
        if (referencedIds.find(desc.id) == referencedIds.end() && rootId.empty())
            rootId = desc.id;
    }
    
    std::function<JComponentHandle(const std::string&, JComponentHandle)> createRecursive;
    createRecursive = [&](const std::string& compId, JComponentHandle parent) -> JComponentHandle {
        auto it = descMap.find(compId);
        if (it == descMap.end()) return JComponentHandle{};
        const auto& desc = *it->second;
        
        JComponentHandle existing = surfaceManager_->findComponent(surfaceId, compId);
        if (existing.isValid()) { updateExistingComponent(existing, desc); return existing; }
        
        JComponentHandle handle = createComponent(desc, parent);
        if (!handle.isValid()) return JComponentHandle{};
        
        surfaceManager_->registerComponent(surfaceId, compId, handle);
        if (compId == rootId) surfaceManager_->setRootComponent(surfaceId, handle);
        
        for (const auto& kv : desc.properties)
            applyProperty(handle, kv.first, kv.second, surfaceId);
        
        if (!desc.child.empty()) createRecursive(desc.child, handle);
        for (const auto& c : desc.children) createRecursive(c, handle);
        return handle;
    };
    
    if (!rootId.empty()) {
        JComponentHandle rh = createRecursive(rootId, JComponentHandle{});
        if (rh.isValid()) { logicLayer_.runFrame(); return rootId; }
    }
    
    for (const auto& desc : descriptors) {
        if (surfaceManager_->findComponent(surfaceId, desc.id).isValid()) continue;
        JComponentHandle h = createComponent(desc, JComponentHandle{});
        if (h.isValid()) {
            surfaceManager_->registerComponent(surfaceId, desc.id, h);
            if (rootId.empty()) rootId = desc.id;
        }
    }
    
    for (const auto& desc : descriptors) {
        JComponentHandle ph = surfaceManager_->findComponent(surfaceId, desc.id);
        if (!ph.isValid()) continue;
        auto* pe = logicLayer_.getStorage().getComponent(ph);
        if (!pe) continue;
        if (!desc.child.empty()) {
            JComponentHandle ch = surfaceManager_->findComponent(surfaceId, desc.child);
            if (ch.isValid()) {
                auto* ce = logicLayer_.getStorage().getComponent(ch);
                if (ce) { ce->parentIndex = ph.index; pe->childrenIndices.push_back(ch.index); }
            }
        }
        for (const auto& cid : desc.children) {
            JComponentHandle ch = surfaceManager_->findComponent(surfaceId, cid);
            if (ch.isValid()) {
                auto* ce = logicLayer_.getStorage().getComponent(ch);
                if (ce) { ce->parentIndex = ph.index; pe->childrenIndices.push_back(ch.index); }
            }
        }
    }
    
    logicLayer_.runFrame();
    return rootId;
}

void JA2UIParser::applyProperty(JComponentHandle handle, const std::string& propName,
                                 const JJSONValue& propValue, const std::string& surfaceId) {
    // 处理数据绑定值: {"path": "/data/path"}
    if (std::holds_alternative<JJSONObject>(propValue.value)) {
        const auto& io = std::get<JJSONObject>(propValue.value);
        auto pi = io.find("path");
        if (pi != io.end() && std::holds_alternative<std::string>(pi->second.value)) {
            JPropertyId pid = getPropertyIdFromName(propName);
            if (pid == JPropertyId::Unknown) {
                // 特殊A2UI属性名到jaether属性名的映射
                if (propName == "variant") pid = JPropertyId::FontSize;
                else if (propName == "justify" || propName == "distribution") pid = JPropertyId::JJustifyContent;
                else if (propName == "align" || propName == "alignment") pid = JPropertyId::JAlignItems;
                else if (propName == "weight") pid = JPropertyId::FlexGrow;
                else if (propName == "label") pid = JPropertyId::Text;
                else if (propName == "value") pid = JPropertyId::Text;
                else if (propName == "primary") pid = JPropertyId::BackgroundColor;
            }
            dataModel_->bindProperty(surfaceId, handle, pid, std::get<std::string>(pi->second.value));
            return;
        }
    }
    
    // A2UI特殊属性名到jaether属性名的映射
    JPropertyId propId = JPropertyId::Unknown;
    if (propName == "text") propId = JPropertyId::Text;
    else if (propName == "width") propId = JPropertyId::Width;
    else if (propName == "height") propId = JPropertyId::Height;
    else if (propName == "weight") propId = JPropertyId::FlexGrow;
    else if (propName == "variant" || propName == "usageHint") propId = JPropertyId::FontSize;
    else if (propName == "justify" || propName == "distribution") propId = JPropertyId::JJustifyContent;
    else if (propName == "align" || propName == "alignment") propId = JPropertyId::JAlignItems;
    else if (propName == "label" || propName == "value") propId = JPropertyId::Text;
    else if (propName == "primary") propId = JPropertyId::BackgroundColor;
    else if (propName == "child" || propName == "children" || propName == "action") return;  // 这些由buildComponentTree处理
    else propId = getPropertyIdFromName(propName);
    
    if (propId == JPropertyId::Unknown) return;
    
    if (propId == JPropertyId::Text) {
        if (std::holds_alternative<std::string>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::get<std::string>(propValue.value)));
    } else if (propId == JPropertyId::Width || propId == JPropertyId::Height ||
               propId == JPropertyId::FlexGrow || propId == JPropertyId::FlexShrink ||
               propId == JPropertyId::FlexBasis) {
        float fv = 0.0f;
        if (std::holds_alternative<float>(propValue.value)) fv = std::get<float>(propValue.value);
        else if (std::holds_alternative<int>(propValue.value)) fv = static_cast<float>(std::get<int>(propValue.value));
        logicLayer_.setProperty(handle, propId, JPropertyValue(fv));
    } else if (propId == JPropertyId::JFlexDirection || propId == JPropertyId::JJustifyContent ||
               propId == JPropertyId::JAlignItems) {
        if (std::holds_alternative<std::string>(propValue.value)) {
            int ev = 0; const std::string& sv = std::get<std::string>(propValue.value);
            if (propId == JPropertyId::JFlexDirection) {
                if (sv == "column" || sv == "Column") ev = 1;
            } else if (propId == JPropertyId::JJustifyContent) {
                if (sv == "center" || sv == "Center") ev = 1;
                else if (sv == "end" || sv == "flexEnd" || sv == "FlexEnd") ev = 2;
                else if (sv == "spaceBetween" || sv == "SpaceBetween") ev = 3;
                else if (sv == "spaceAround" || sv == "SpaceAround") ev = 4;
            } else if (propId == JPropertyId::JAlignItems) {
                if (sv == "center" || sv == "Center") ev = 1;
                else if (sv == "end" || sv == "flexEnd" || sv == "FlexEnd") ev = 2;
                else if (sv == "stretch" || sv == "Stretch") ev = 3;
            }
            logicLayer_.setProperty(handle, propId, JPropertyValue(ev));
        }
    } else if (propId == JPropertyId::FontSize) {
        float fs = 16.0f;
        if (std::holds_alternative<std::string>(propValue.value)) {
            const std::string& v = std::get<std::string>(propValue.value);
            if (v == "h1") fs = 32.0f; else if (v == "h2") fs = 28.0f;
            else if (v == "h3") fs = 24.0f; else if (v == "h4") fs = 20.0f;
            else if (v == "h5") fs = 16.0f; else if (v == "body") fs = 14.0f;
            else if (v == "caption") fs = 12.0f;
        }
        logicLayer_.setProperty(handle, propId, JPropertyValue(fs));
    } else if (propId == JPropertyId::BackgroundColor) {
        if (std::holds_alternative<std::string>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::get<std::string>(propValue.value)));
        else if (std::holds_alternative<bool>(propValue.value) && std::get<bool>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::string("primary")));
    } else {
        if (std::holds_alternative<std::string>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::get<std::string>(propValue.value)));
        else if (std::holds_alternative<int>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::get<int>(propValue.value)));
        else if (std::holds_alternative<float>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::get<float>(propValue.value)));
        else if (std::holds_alternative<bool>(propValue.value))
            logicLayer_.setProperty(handle, propId, JPropertyValue(std::get<bool>(propValue.value)));
    }
}

void JA2UIParser::applyComponentUpdate(const std::string& updateJson, const std::string& surfaceId) {
    JJSONValue root = JJSONParser::parse(updateJson);
    if (!std::holds_alternative<JJSONObject>(root.value)) return;
    const auto& ro = std::get<JJSONObject>(root.value);
    auto ui = ro.find("updateComponents");
    if (ui == ro.end()) ui = ro.find("surfaceUpdate");
    if (ui == ro.end() || !std::holds_alternative<JJSONObject>(ui->second.value)) return;
    const auto& uo = std::get<JJSONObject>(ui->second.value);
    std::string sid = surfaceId;
    auto si = uo.find("surfaceId");
    if (si != uo.end() && std::holds_alternative<std::string>(si->second.value))
        sid = std::get<std::string>(si->second.value);
    auto ci = uo.find("components");
    if (ci == uo.end() || !std::holds_alternative<JJSONArray>(ci->second.value)) return;
    logicLayer_.beginBatch();
    for (const auto& c : std::get<JJSONArray>(ci->second.value)) {
        if (!std::holds_alternative<JJSONObject>(c.value)) continue;
        const auto& co = std::get<JJSONObject>(c.value);
        ComponentDescriptor desc;
        auto cf = co.find("component");
        if (cf != co.end() && std::holds_alternative<JJSONObject>(cf->second.value))
            desc = parseV08Component(co);
        else
            desc = parseDescriptor(co);
        JComponentHandle ex = surfaceManager_->findComponent(sid, desc.id);
        if (ex.isValid()) updateExistingComponent(ex, desc);
    }
    logicLayer_.endBatch();
}

void JA2UIParser::applyDataModelUpdate(const std::string& dataJson, const std::string&) {
    JJSONValue root = JJSONParser::parse(dataJson);
    if (!std::holds_alternative<JJSONObject>(root.value)) return;
    const auto& ro = std::get<JJSONObject>(root.value);
    auto di = ro.find("updateDataModel");
    if (di == ro.end()) di = ro.find("dataModelUpdate");
    if (di != ro.end() && std::holds_alternative<JJSONObject>(di->second.value))
        handleUpdateDataModel(std::get<JJSONObject>(di->second.value));
}

void JA2UIParser::parseJSONL(const std::string& jsonl) {
    std::istringstream ss(jsonl);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty() || line[0] != '{') continue;
        JJSONValue r = JJSONParser::parse(line);
        if (!std::holds_alternative<JJSONObject>(r.value)) continue;
        const auto& ro = std::get<JJSONObject>(r.value);
        auto ci = ro.find("createSurface");
        if (ci != ro.end() && std::holds_alternative<JJSONObject>(ci->second.value))
            { handleCreateSurface(std::get<JJSONObject>(ci->second.value)); continue; }
        auto ui = ro.find("updateComponents");
        if (ui != ro.end() && std::holds_alternative<JJSONObject>(ui->second.value))
            { handleUpdateComponents(std::get<JJSONObject>(ui->second.value)); continue; }
        auto di = ro.find("updateDataModel");
        if (di != ro.end() && std::holds_alternative<JJSONObject>(di->second.value))
            { handleUpdateDataModel(std::get<JJSONObject>(di->second.value)); continue; }
        auto dsi = ro.find("deleteSurface");
        if (dsi != ro.end() && std::holds_alternative<JJSONObject>(dsi->second.value)) {
            auto si = std::get<JJSONObject>(dsi->second.value).find("surfaceId");
            if (si != std::get<JJSONObject>(dsi->second.value).end() &&
                std::holds_alternative<std::string>(si->second.value))
                surfaceManager_->deleteSurface(std::get<std::string>(si->second.value));
        }
    }
}

} // namespace jaether
