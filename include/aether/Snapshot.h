// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


#pragma once

#include "aether/types.h"
#include "aether/ComponentStorage.h"
#include <vector>
#include <string>

namespace jaether {

/**
 * 快照中的组件信息结构
 * 
 * 保存组件的关键状态信息
 */
struct JSnapshotComponent {
    JComponentId id;                // 组件ID
    JComponentType type;          // 组件类型
    JRect layout;                 // 布局信息
    int32_t parentId;              // 父组件ID
    std::vector<int32_t> childrenIds;  // 子组件ID列表
    bool visible;               // 是否可见
    bool enabled;               // 是否启用
    std::string debugName;       // 调试名称
};

/**
 * 快照结构
 * 
 * 保存完整的UI状态快照
 * 用于测试、调试和状态恢复
 */
struct JSnapshot {
    std::vector<JSnapshotComponent> components;  // 所有组件信息
    uint64_t timestamp = 0;                      // 时间戳
    int componentCount = 0;                      // 组件数量
    std::string version;                        // 版本号
};

/**
 * 快照序列化工具类
 * 
 * 提供快照的创建、应用、序列化和比较功能
 */
class JSnapshotSerializer {
public:
    /**
     * 从组件存储创建快照
     * @param storage 组件存储
     * @return 快照对象
     */
    static JSnapshot capture(const JComponentStorage& storage);
    
    /**
     * 将快照应用到组件存储
     * @param snapshot 快照
     * @param storage 目标组件存储
     */
    static void apply(const JSnapshot& snapshot, JComponentStorage& storage);
    
    /**
     * 将快照序列化为JSON字符串
     * @param snapshot 快照
     * @return JSON字符串
     */
    static std::string toJSON(const JSnapshot& snapshot);
    
    /**
     * 从JSON字符串反序列化快照
     * @param json JSON字符串
     * @return 快照对象
     */
    static JSnapshot fromJSON(const std::string& json);
    
    /**
     * 将快照序列化为二进制数据
     * @param snapshot 快照
     * @return 二进制数据
     */
    static std::vector<uint8_t> toBinary(const JSnapshot& snapshot);
    
    /**
     * 从二进制数据反序列化快照
     * @param data 二进制数据
     * @return 快照对象
     */
    static JSnapshot fromBinary(const std::vector<uint8_t>& data);
    
    /**
     * 比较两个快照是否相同
     * @param a 快照a
     * @param b 快照b
     * @return 如果相同返回true
     */
    static bool compare(const JSnapshot& a, const JSnapshot& b);
    
    /**
     * 生成两个快照的差异报告
     * @param a 快照a
     * @param b 快照b
     * @return 差异字符串
     */
    static std::string diff(const JSnapshot& a, const JSnapshot& b);
};

}
