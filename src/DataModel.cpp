// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// DataModel.cpp
// 数据模型与绑定模块 - 管理A2UI协议的数据模型和属性绑定
//
// 功能:
// - 存储和管理A2UI数据模型（树形JSON结构）
// - 支持通过路径（如"/user/name"）读写数据
// - 注册组件属性到数据路径的绑定关系
// - 当数据变更时自动通知所有绑定组件
// - 导出数据模型为A2UI格式JSON

#include "aether/DataModel.h"
#include <sstream>
#include <algorithm>

namespace jaether {

// 构造函数
JJDataModel::JJDataModel() {
}

// 设置属性绑定回调函数
// 当绑定的数据路径变更时，此回调被触发来更新组件属性
void JJDataModel::setBindingCallback(BindingCallback callback) {
    bindingCallback_ = std::move(callback);
}

// 将路径字符串解析为路径段数组
// 处理路径格式: "/user/name" → ["user", "name"]
// 处理空路径: "" 或 "/" → []
// 自动去除首尾斜杠
std::vector<std::string> JJDataModel::parsePath(const std::string& path) const {
    std::vector<std::string> segments;
    if (path.empty()) {
        return segments;
    }
    
    std::string cleanPath = path;
    
    // 去除开头的斜杠
    if (!cleanPath.empty() && cleanPath[0] == '/') {
        cleanPath = cleanPath.substr(1);
    }
    
    // 去除结尾的斜杠
    if (!cleanPath.empty() && cleanPath.back() == '/') {
        cleanPath.pop_back();
    }
    
    // 如果清理后为空，返回空数组
    if (cleanPath.empty()) {
        return segments;
    }
    
    // 按斜杠分割路径
    size_t start = 0;
    size_t end = cleanPath.find('/');
    while (end != std::string::npos) {
        std::string segment = cleanPath.substr(start, end - start);
        if (!segment.empty()) {
            segments.push_back(segment);
        }
        start = end + 1;
        end = cleanPath.find('/', start);
    }
    
    // 处理最后一段
    std::string lastSegment = cleanPath.substr(start);
    if (!lastSegment.empty()) {
        segments.push_back(lastSegment);
    }
    
    return segments;
}

// 应用A2UI数据模型更新消息
// contents数组中的每一项包含"key"字段和值字段(valueString/valueNumber/valueBoolean/valueMap)
// 当path为空时，更新作用于根级别
void JJDataModel::applyUpdate(const std::string& surfaceId, 
                               const std::string& path, 
                               const JJSONArray& contents) {
    // 获取该Surface的数据模型根对象（如果不存在则创建空对象）
    auto& modelRoot = dataModels_[surfaceId];
    
    // 解析目标路径
    auto pathSegments = parsePath(path);
    
    // 遍历contents数组中的每个数据条目
    for (const auto& item : contents) {
        // 每个条目必须是JJSONObject类型
        if (!std::holds_alternative<JJSONObject>(item.value)) {
            continue;
        }
        
        const auto& entry = std::get<JJSONObject>(item.value);
        
        // 提取key字段（在A2UI v0.9中为"key"）
        auto keyIt = entry.find("key");
        if (keyIt == entry.end()) {
            continue;  // 缺少key字段，跳过此条目
        }
        
        std::string key;
        if (std::holds_alternative<std::string>(keyIt->second.value)) {
            key = std::get<std::string>(keyIt->second.value);
        } else {
            continue;  // key不是字符串类型，跳过
        }
        
        // 构建完整路径：基础路径 + key
        auto fullPath = pathSegments;
        fullPath.push_back(key);
        
        // 提取值字段（优先级: valueString > valueNumber > valueBoolean > valueMap）
        JJSONValue value;
        bool foundValue = false;
        
        // 检查 valueString
        auto valStrIt = entry.find("valueString");
        if (valStrIt != entry.end()) {
            value = valStrIt->second;
            foundValue = true;
        }
        
        // 检查 valueNumber（如果valueString不存在）
        if (!foundValue) {
            auto valNumIt = entry.find("valueNumber");
            if (valNumIt != entry.end()) {
                value = valNumIt->second;
                foundValue = true;
            }
        }
        
        // 检查 valueBoolean（如果前两者都不存在）
        if (!foundValue) {
            auto valBoolIt = entry.find("valueBoolean");
            if (valBoolIt != entry.end()) {
                value = valBoolIt->second;
                foundValue = true;
            }
        }
        
        // 检查 valueMap（如果前面都不存在）
        if (!foundValue) {
            auto valMapIt = entry.find("valueMap");
            if (valMapIt != entry.end()) {
                value = valMapIt->second;
                foundValue = true;
            }
        }
        
        // 如果没有找到任何值字段，跳过
        if (!foundValue) {
            continue;
        }
        
        // 设置值到数据模型
        setValueAtPath(surfaceId, fullPath, value);
        
        // 重建路径字符串用于通知
        std::string fullPathStr;
        for (const auto& seg : fullPath) {
            fullPathStr += "/" + seg;
        }
        
        // 通知绑定到该路径的所有组件
        notifyBindings(surfaceId, fullPathStr);
    }
}

// 获取数据模型中指定路径的值
JJSONValue JJDataModel::getValue(const std::string& surfaceId, const std::string& path) const {
    auto pathSegments = parsePath(path);
    return getValueAtPath(surfaceId, pathSegments);
}

// 设置数据模型中指定路径的值
void JJDataModel::setValue(const std::string& surfaceId, const std::string& path, const JJSONValue& value) {
    auto pathSegments = parsePath(path);
    setValueAtPath(surfaceId, pathSegments, value);
    
    // 通知绑定组件
    notifyBindings(surfaceId, path);
}

// 在数据模型中设置值（内部实现）
// 递归遍历路径，自动创建不存在的中间节点
void JJDataModel::setValueAtPath(const std::string& surfaceId, 
                                  const std::vector<std::string>& pathSegments, 
                                  const JJSONValue& value) {
    if (pathSegments.empty()) {
        return;
    }
    
    auto& modelRoot = dataModels_[surfaceId];
    
    // 遍历路径直到倒数第二层
    JJSONObject* current = &modelRoot;
    for (size_t i = 0; i < pathSegments.size() - 1; ++i) {
        const std::string& segment = pathSegments[i];
        
        // 检查当前层级是否存在该key
        auto it = current->find(segment);
        if (it == current->end()) {
            // 创建中间节点（JJSONObject）
            (*current)[segment] = JJSONValue(JJSONObject{});
            it = current->find(segment);
        }
        
        // 获取下一层对象引用
        if (std::holds_alternative<JJSONObject>(it->second.value)) {
            current = &std::get<JJSONObject>(it->second.value);
        } else {
            // 如果当前节点不是对象类型，覆盖为对象
            it->second = JJSONValue(JJSONObject{});
            current = &std::get<JJSONObject>(it->second.value);
        }
    }
    
    // 设置最后一层的值
    const std::string& lastSegment = pathSegments.back();
    (*current)[lastSegment] = value;
}

// 从数据模型中获取值（内部实现）
JJSONValue JJDataModel::getValueAtPath(const std::string& surfaceId,
                                         const std::vector<std::string>& pathSegments) const {
    auto modelIt = dataModels_.find(surfaceId);
    if (modelIt == dataModels_.end()) {
        return JJSONValue();  // 数据模型不存在，返回null
    }
    
    const JJSONObject* current = &modelIt->second;
    
    for (size_t i = 0; i < pathSegments.size(); ++i) {
        const std::string& segment = pathSegments[i];
        
        auto it = current->find(segment);
        if (it == current->end()) {
            return JJSONValue();  // 路径不存在，返回null
        }
        
        // 如果是最后一段，返回该值
        if (i == pathSegments.size() - 1) {
            return it->second;
        }
        
        // 否则继续向下遍历（当前节点必须是对象类型）
        if (std::holds_alternative<JJSONObject>(it->second.value)) {
            current = &std::get<JJSONObject>(it->second.value);
        } else {
            return JJSONValue();  // 中间节点不是对象，路径无效
        }
    }
    
    return JJSONValue();
}

// 注册组件属性的数据绑定
// 建立 数据路径 → 组件属性 的绑定关系
void JJDataModel::bindProperty(const std::string& surfaceId, 
                                JComponentHandle handle, 
                                JPropertyId propId, 
                                const std::string& dataPath) {
    if (!handle.isValid()) {
        return;
    }
    
    PropertyBinding binding;
    binding.handle = handle;
    binding.propId = propId;
    
    bindings_[surfaceId][dataPath].push_back(binding);
}

// 解除组件的属性绑定
// 从指定路径的绑定列表中移除该组件
void JJDataModel::unbindProperty(const std::string& surfaceId, 
                                  JComponentHandle handle, 
                                  const std::string& dataPath) {
    auto surfaceIt = bindings_.find(surfaceId);
    if (surfaceIt == bindings_.end()) {
        return;
    }
    
    auto pathIt = surfaceIt->second.find(dataPath);
    if (pathIt == surfaceIt->second.end()) {
        return;
    }
    
    // 从绑定列表中移除匹配的绑定
    auto& bindings = pathIt->second;
    bindings.erase(
        std::remove_if(bindings.begin(), bindings.end(),
            [&handle](const PropertyBinding& b) {
                return b.handle.index == handle.index && 
                       b.handle.generation == handle.generation;
            }),
        bindings.end()
    );
    
    // 如果绑定列表为空，也清理路径条目
    if (pathIt->second.empty()) {
        surfaceIt->second.erase(pathIt);
    }
}

// 通知所有绑定到指定路径的组件数据已变更
void JJDataModel::notifyBindings(const std::string& surfaceId, const std::string& changedPath) {
    if (!bindingCallback_) {
        return;  // 没有设置回调函数，无法通知
    }
    
    // 查找该路径的所有绑定
    auto surfaceIt = bindings_.find(surfaceId);
    if (surfaceIt == bindings_.end()) {
        return;
    }
    
    auto pathIt = surfaceIt->second.find(changedPath);
    if (pathIt == surfaceIt->second.end()) {
        return;
    }
    
    // 获取当前路径的新值
    JJSONValue newValue = getValue(surfaceId, changedPath);
    
    // 通知每个绑定组件
    for (const auto& binding : pathIt->second) {
        bindingCallback_(binding.handle, binding.propId, newValue);
    }
}

// 导出数据模型为A2UI JSON格式字符串
// 将整个数据模型序列化为A2UI兼容的JSON格式
std::string JJDataModel::exportJSON(const std::string& surfaceId) const {
    auto modelIt = dataModels_.find(surfaceId);
    if (modelIt == dataModels_.end()) {
        return "{}";
    }
    
    JJSONValue rootValue(modelIt->second);
    return JJSONParser::stringify(rootValue);
}

// 检查指定Surface是否有数据模型
bool JJDataModel::hasDataModel(const std::string& surfaceId) const {
    return dataModels_.find(surfaceId) != dataModels_.end();
}

// 清空指定Surface的数据模型
void JJDataModel::clearDataModel(const std::string& surfaceId) {
    dataModels_.erase(surfaceId);
    bindings_.erase(surfaceId);
}

// 检查指定路径是否存在
bool JJDataModel::pathExists(const std::string& surfaceId, const std::string& path) const {
    JJSONValue val = getValue(surfaceId, path);
    return !std::holds_alternative<std::nullptr_t>(val.value);
}

} // namespace jaether
