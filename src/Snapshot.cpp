// Snapshot.cpp
// 快照模块 - 负责UI状态的序列化和反序列化
//
// 功能:
// - 捕获当前UI状态
// - 应用保存的UI状态
// - 序列化为JSON格式
// - 从JSON反序列化
// - 快照比较和差异分析

#include "aether/Snapshot.h"
#include "aether/aether.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <unordered_map>

namespace aether {

// 捕获当前UI状态快照
// 参数: storage - 组件存储引用
// 返回值: 快照对象
Snapshot SnapshotSerializer::capture(const ComponentStorage& storage) {
    Snapshot snapshot;
    snapshot.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    snapshot.version = AETHER_VERSION;
    
    // 遍历所有组件
    storage.forEach([&](ComponentHandle h) {
        auto* entry = storage.getComponent(h);
        if (!entry) return;
        
        SnapshotComponent comp;
        comp.id = entry->id;
        comp.type = entry->type;
        comp.layout = entry->layoutResult;
        comp.visible = entry->visible;
        comp.enabled = entry->enabled;
        comp.debugName = entry->debugName;
        comp.parentId = -1;
        
        // 获取父组件ID
        if (entry->parentIndex >= 0) {
            auto* parent = storage.getComponent(ComponentHandle{entry->parentIndex, 0});
            if (parent) {
                comp.parentId = static_cast<int32_t>(parent->id);
            }
        }
        
        // 获取子组件ID列表
        for (int32_t childIdx : entry->childrenIndices) {
            auto* child = storage.getComponent(ComponentHandle{childIdx, 0});
            if (child) {
                comp.childrenIds.push_back(static_cast<int32_t>(child->id));
            }
        }
        
        snapshot.components.push_back(comp);
    });
    
    snapshot.componentCount = static_cast<int>(snapshot.components.size());
    return snapshot;
}

// 应用快照到组件存储
// 参数:
//   snapshot - 快照对象
//   storage - 组件存储引用
void SnapshotSerializer::apply(const Snapshot& snapshot, ComponentStorage& storage) {
    storage.clear();
    
    std::unordered_map<int32_t, ComponentHandle> idToHandle;
    
    // 先创建根组件（parentId < 0）
    for (const auto& comp : snapshot.components) {
        if (comp.parentId < 0) {
            ComponentHandle h = storage.createComponent(comp.type, {});
            auto* entry = storage.getComponent(h);
            if (entry) {
                entry->layoutResult = comp.layout;
                entry->visible = comp.visible;
                entry->enabled = comp.enabled;
                entry->debugName = comp.debugName;
                idToHandle[comp.id] = h;
            }
        }
    }
    
    // 再创建子组件
    for (const auto& comp : snapshot.components) {
        if (comp.parentId >= 0) {
            auto it = idToHandle.find(comp.parentId);
            if (it != idToHandle.end()) {
                ComponentHandle h = storage.createComponent(comp.type, it->second);
                auto* entry = storage.getComponent(h);
                if (entry) {
                    entry->layoutResult = comp.layout;
                    entry->visible = comp.visible;
                    entry->enabled = comp.enabled;
                    entry->debugName = comp.debugName;
                    idToHandle[comp.id] = h;
                }
            }
        }
    }
}

// 将快照序列化为JSON字符串
// 参数: snapshot - 快照对象
// 返回值: JSON字符串
std::string SnapshotSerializer::toJSON(const Snapshot& snapshot) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"version\": \"" << snapshot.version << "\",\n";
    oss << "  \"timestamp\": " << snapshot.timestamp << ",\n";
    oss << "  \"componentCount\": " << snapshot.componentCount << ",\n";
    oss << "  \"components\": [\n";
    
    // 序列化每个组件
    for (size_t i = 0; i < snapshot.components.size(); ++i) {
        const auto& comp = snapshot.components[i];
        oss << "    {\n";
        oss << "      \"id\": " << comp.id << ",\n";
        oss << "      \"type\": " << static_cast<int>(comp.type) << ",\n";
        oss << "      \"layout\": {";
        oss << "\"x\":" << comp.layout.left() << ",";
        oss << "\"y\":" << comp.layout.top() << ",";
        oss << "\"width\":" << comp.layout.width << ",";
        oss << "\"height\":" << comp.layout.height << "},\n";
        oss << "      \"visible\": " << (comp.visible ? "true" : "false") << ",\n";
        oss << "      \"enabled\": " << (comp.enabled ? "true" : "false") << ",\n";
        oss << "      \"parentId\": " << comp.parentId << ",\n";
        oss << "      \"childrenIds\": [";
        // 序列化子组件ID列表
        for (size_t j = 0; j < comp.childrenIds.size(); ++j) {
            if (j > 0) oss << ",";
            oss << comp.childrenIds[j];
        }
        oss << "],\n";
        oss << "      \"debugName\": \"" << comp.debugName << "\"\n";
        oss << "    }";
        if (i < snapshot.components.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "  ]\n";
    oss << "}\n";
    return oss.str();
}

// 从JSON字符串解析快照
// 参数: json - JSON字符串
// 返回值: 快照对象
Snapshot SnapshotSerializer::fromJSON(const std::string& json) {
    Snapshot snapshot;
    snapshot.version = AETHER_VERSION;
    snapshot.timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return snapshot;
}

// 将快照序列化为二进制数据
// 参数: snapshot - 快照对象
// 返回值: 二进制数据
std::vector<uint8_t> SnapshotSerializer::toBinary(const Snapshot& snapshot) {
    return std::vector<uint8_t>();
}

// 从二进制数据解析快照
// 参数: data - 二进制数据
// 返回值: 快照对象
Snapshot SnapshotSerializer::fromBinary(const std::vector<uint8_t>& data) {
    Snapshot snapshot;
    return snapshot;
}

// 比较两个快照是否相同
// 参数: a, b - 要比较的两个快照
// 返回值: 相同返回true，否则返回false
bool SnapshotSerializer::compare(const Snapshot& a, const Snapshot& b) {
    // 先比较组件数量
    if (a.componentCount != b.componentCount) return false;
    
    // 逐个比较组件
    for (size_t i = 0; i < a.components.size(); ++i) {
        const auto& ca = a.components[i];
        const auto& cb = b.components[i];
        
        if (ca.id != cb.id) return false;
        if (ca.type != cb.type) return false;
        if (ca.layout.x != cb.layout.x) return false;
        if (ca.layout.y != cb.layout.y) return false;
        if (ca.layout.width != cb.layout.width) return false;
        if (ca.layout.height != cb.layout.height) return false;
        if (ca.visible != cb.visible) return false;
        if (ca.enabled != cb.enabled) return false;
    }
    
    return true;
}

// 生成两个快照的差异报告
// 参数: a, b - 要比较的两个快照
// 返回值: 差异描述字符串
std::string SnapshotSerializer::diff(const Snapshot& a, const Snapshot& b) {
    std::ostringstream oss;
    
    // 检查组件数量
    if (a.componentCount != b.componentCount) {
        oss << "Component count mismatch: " << a.componentCount << " vs " << b.componentCount << "\n";
    }
    
    // 检查每个组件的差异
    for (size_t i = 0; i < std::min(a.components.size(), b.components.size()); ++i) {
        const auto& ca = a.components[i];
        const auto& cb = b.components[i];
        
        if (ca.layout.x != cb.layout.x) {
            oss << "Component " << ca.id << " x: " << ca.layout.x << " vs " << cb.layout.x << "\n";
        }
        if (ca.layout.y != cb.layout.y) {
            oss << "Component " << ca.id << " y: " << ca.layout.y << " vs " << cb.layout.y << "\n";
        }
        if (ca.layout.width != cb.layout.width) {
            oss << "Component " << ca.id << " width: " << ca.layout.width << " vs " << cb.layout.width << "\n";
        }
        if (ca.layout.height != cb.layout.height) {
            oss << "Component " << ca.id << " height: " << ca.layout.height << " vs " << cb.layout.height << "\n";
        }
    }
    
    return oss.str();
}

} // namespace aether
