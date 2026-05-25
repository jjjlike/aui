#pragma once

#include "LogicLayer.h"
#include "ComponentCatalog.h"
#include "SurfaceManager.h"
#include "DataModel.h"
#include "JSONParser.h"
#include <string>
#include <vector>
#include <memory>

namespace jaether {

/**
 * A2UI JSON解析器类 — JSON到组件引擎
 * 
 * 将A2UI协议的JSON描述解析为jaether组件树
 * 支持v0.9和v0.8两种协议格式
 * 支持完整surface初始化、增量更新、数据模型更新、JSONL流式解析
 */
class JA2UIParser {
public:
    /**
     * 构造函数
     * @param logicLayer jaether逻辑层引用，用于创建和管理组件
     */
    explicit JA2UIParser(JLogicLayer& logicLayer);

    /**
     * 解析完整的A2UI JSON字符串并创建所有组件
     * 自动识别消息类型（createSurface/updateComponents/updateDataModel）
     * @param json A2UI格式的JSON字符串
     * @param surfaceId 目标Surface标识符，如果JSON中包含createSurface则以此参数为准
     * @return 创建的根组件A2UI ID，失败返回空字符串
     */
    std::string parseAndApply(const std::string& json, const std::string& surfaceId);

    /**
     * 解析并应用单条updateComponents消息（v0.9格式）
     * 支持增量更新：如果组件ID已存在则更新属性，否则创建新组件
     * @param updateJson 单条updateComponents消息的JSON字符串
     * @param surfaceId 目标Surface标识符
     */
    void applyComponentUpdate(const std::string& updateJson, const std::string& surfaceId);

    /**
     * 解析并应用单条updateDataModel消息
     * @param dataJson 数据模型更新的JSON字符串
     * @param surfaceId 目标Surface标识符
     */
    void applyDataModelUpdate(const std::string& dataJson, const std::string& surfaceId);

    /**
     * 解析JSONL格式的多行消息流
     * 每行一个完整的JSON消息，逐行解析并应用
     * @param jsonl JSONL格式字符串
     */
    void parseJSONL(const std::string& jsonl);

    /**
     * 获取组件目录注册表
     * @return 组件目录注册表引用
     */
    JJComponentCatalog& getCatalog() { return *catalog_; }

    /**
     * 获取Surface管理器
     * @return Surface管理器引用
     */
    JJSurfaceManager& getSurfaceManager() { return *surfaceManager_; }

    /**
     * 获取数据模型
     * @return 数据模型引用
     */
    JJDataModel& getDataModel() { return *dataModel_; }

private:
    /**
     * 组件描述符结构体
     * JSON中间表示，存储解析后的组件信息
     */
    struct ComponentDescriptor {
        std::string id;                      // A2UI组件ID
        std::string type;                    // A2UI组件类型名称
        std::string child;                   // 单个子组件ID（用于Button/Card等）
        std::vector<std::string> children;   // 多个子组件ID列表（用于Row/Column等）
        JJSONObject properties;              // 属性键值对
        bool hasDataBinding = false;         // 是否包含数据绑定属性
    };

    /**
     * 处理单条A2UI消息
     * 根据消息类型分发给对应的处理方法
     * @param messageJson 单条消息的JSON字符串
     */
    void processMessage(const std::string& messageJson);

    /**
     * 解析更新组件消息（v0.9格式）
     * @param updateObj updateComponents消息的JSON对象
     */
    std::string handleUpdateComponents(const JJSONObject& updateObj);

    /**
     * 解析创建Surface消息（v0.9格式）
     * @param createObj createSurface消息的JSON对象
     */
    void handleCreateSurface(const JJSONObject& createObj);

    /**
     * 解析数据模型更新消息
     * @param dataObj updateDataModel消息的JSON对象
     */
    void handleUpdateDataModel(const JJSONObject& dataObj);

    /**
     * 解析单个组件描述符
     * @param obj 组件的JSON对象
     * @return 解析后的组件描述符
     */
    ComponentDescriptor parseDescriptor(const JJSONObject& obj);

    /**
     * 解析v0.8格式的组件并转换为v0.9格式描述符
     * @param obj v0.8格式的组件JSON对象
     * @return v0.9格式的组件描述符
     */
    ComponentDescriptor parseV08Component(const JJSONObject& obj);

    /**
     * 根据描述符创建jaether组件
     * @param desc 组件描述符
     * @param parent 父组件句柄
     * @return 新创建的组件句柄
     */
    JComponentHandle createComponent(const ComponentDescriptor& desc, JComponentHandle parent);

    /**
     * 在已有组件上应用属性更新
     * @param handle 已有组件句柄
     * @param desc 包含新属性的描述符
     */
    void updateExistingComponent(JComponentHandle handle, const ComponentDescriptor& desc);

    /**
     * 设置组件的单个属性
     * 处理类型转换、variant映射、数据绑定等
     * @param handle 组件句柄
     * @param propName A2UI属性名称
     * @param propValue 属性JSON值
     * @param surfaceId 所属Surface ID
     */
    void applyProperty(JComponentHandle handle, const std::string& propName, 
                       const JJSONValue& propValue, const std::string& surfaceId);

    /**
     * 递归创建组件树（拓扑排序后按序创建）
     * @param descriptors 组件描述符列表
     * @param surfaceId 目标Surface ID
     * @return 根组件的A2UI ID
     */
    std::string buildComponentTree(const std::vector<ComponentDescriptor>& descriptors,
                                    const std::string& surfaceId);

    /**
     * 解析子节点引用（explicitList格式）
     * @param childrenValue children属性的JSON值
     * @return 子组件A2UI ID列表
     */
    std::vector<std::string> parseChildrenList(const JJSONValue& childrenValue);

    /**
     * 将v0.8 BoundValue转换为v0.9格式
     * {"literalString": "hello"} → "hello"
     * {"path": "/data"} → {"path": "/data"} (保持不变)
     * @param value v0.8格式的BoundValue
     * @return v0.9格式的值
     */
    JJSONValue convertV08BoundValue(const JJSONValue& value);

    JLogicLayer& logicLayer_;                               // 逻辑层引用
    std::unique_ptr<JJComponentCatalog> catalog_;           // 组件目录注册表
    std::unique_ptr<JJSurfaceManager> surfaceManager_;      // Surface管理器
    std::unique_ptr<JJDataModel> dataModel_;                // 数据模型
};

} // namespace jaether
