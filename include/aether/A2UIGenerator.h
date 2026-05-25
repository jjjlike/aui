#pragma once

#include "LogicLayer.h"
#include "ComponentCatalog.h"
#include "SurfaceManager.h"
#include "JSONParser.h"
#include <string>
#include <memory>

namespace jaether {

/**
 * A2UI JSON生成器类 — 组件到JSON引擎
 * 
 * 将jaether组件树导出为A2UI协议的JSON描述
 * 支持完整Surface导出、单组件导出、数据模型导出
 * 与JA2UIParser配对使用，实现双向转换
 */
class JA2UIGenerator {
public:
    /**
     * 构造函数
     * @param logicLayer jaether逻辑层引用，用于读取组件状态
     * @param surfaceManager Surface管理器引用，用于ID映射
     */
    explicit JA2UIGenerator(const JLogicLayer& logicLayer, const JJSurfaceManager& surfaceManager);

    /**
     * 生成完整Surface的A2UI JSON描述
     * 输出格式为v0.9 updateComponents消息
     * @param surfaceId 目标Surface标识符
     * @return A2UI JSON字符串，Surface不存在返回"{}"
     */
    std::string generateSurfaceJSON(const std::string& surfaceId) const;

    /**
     * 生成单个组件的A2UI JSON描述
     * @param handle 组件句柄
     * @param a2uiId 组件的A2UI ID
     * @return 组件描述JSON对象
     */
    JJSONObject generateComponentJSON(JComponentHandle handle, const std::string& a2uiId) const;

    /**
     * 生成数据模型的A2UI JSON
     * @param surfaceId 目标Surface标识符
     * @return A2UI格式的数据模型JSON字符串
     */
    std::string generateDataModelJSON(const std::string& surfaceId) const;

    /**
     * 输出A2UI格式的组件树JSON
     * 与TestController::getComponentTreeJSON互补
     * @return A2UI格式的组件树JSON字符串
     */
    std::string getComponentTreeA2UI() const;

private:
    /**
     * 将jaether属性值转换为A2UI JSON值
     * 处理类型逆向映射：枚举→字符串、字号→variant等
     * @param propId 属性ID
     * @param value 属性值
     * @return A2UI格式的JSON值
     */
    JJSONValue propertyToA2UIValue(JPropertyId propId, const JPropertyValue& value) const;

    /**
     * 将子组件索引列表转换为A2UI ID字符串列表
     * @param childrenIndices 子组件索引列表
     * @param surfaceId 目标Surface标识符
     * @return A2UI组件ID字符串列表
     */
    std::vector<std::string> childrenToA2UIIDs(const std::vector<int32_t>& childrenIndices,
                                                const std::string& surfaceId) const;

    /**
     * 根据字号推理variant名称
     * @param fontSize 字号（float）
     * @return variant名称字符串
     */
    std::string fontSizeToVariant(float fontSize) const;

    /**
     * 根据对齐枚举值推理对齐字符串
     * @param enumVal 枚举值
     * @param isJustify true表示主轴对齐，false表示交叉轴对齐
     * @return 对齐字符串
     */
    std::string alignmentToString(int enumVal, bool isJustify) const;

    const JLogicLayer& logicLayer_;              // 逻辑层引用
    const JJSurfaceManager& surfaceManager_;     // Surface管理器引用
    std::unique_ptr<JJComponentCatalog> catalog_; // 组件目录注册表
};

} // namespace jaether
