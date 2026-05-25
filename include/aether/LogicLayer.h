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

#include "types.h"
#include "ComponentStorage.h"
#include "LayoutEngine.h"
#include "EventDispatcher.h"
#include "StateManager.h"
#include "Snapshot.h"
#include "TestController.h"
#include <memory>

namespace jaether {

// 前向声明A2UI模块
class JA2UIParser;
class JA2UIGenerator;
class JJSurfaceManager;
class JJDataModel;

/**
 * 逻辑层类
 * 
 * 整合所有核心组件的高层接口
 * 提供统一的访问点，简化使用
 * 是框架的核心协调者
 */
class JLogicLayer {
public:
    /**
     * 构造函数
     */
    JLogicLayer();
    
    /**
     * 析构函数
     */
    ~JLogicLayer();
    
    /**
     * 获取组件存储（可变版本）
     * @return 组件存储引用
     */
    JComponentStorage& getStorage() { return storage_; }
    
    /**
     * 获取组件存储（const版本）
     * @return 组件存储const引用
     */
    const JComponentStorage& getStorage() const { return storage_; }
    
    /**
     * 获取布局引擎（可变版本）
     * @return 布局引擎引用
     */
    JLayoutEngine& getLayoutEngine() { return layoutEngine_; }
    
    /**
     * 获取布局引擎（const版本）
     * @return 布局引擎const引用
     */
    const JLayoutEngine& getLayoutEngine() const { return layoutEngine_; }
    
    /**
     * 获取事件分发器（可变版本）
     * @return 事件分发器引用
     */
    JEventDispatcher& getEventDispatcher() { return eventDispatcher_; }
    
    /**
     * 获取事件分发器（const版本）
     * @return 事件分发器const引用
     */
    const JEventDispatcher& getEventDispatcher() const { return eventDispatcher_; }
    
    /**
     * 获取状态管理器（可变版本）
     * @return 状态管理器引用
     */
    JStateManager& getStateManager() { return stateManager_; }
    
    /**
     * 获取状态管理器（const版本）
     * @return 状态管理器const引用
     */
    const JStateManager& getStateManager() const { return stateManager_; }
    
    /**
     * 获取测试控制器（可变版本）
     * @return 测试控制器引用
     */
    JTestController& getTestController() { return testController_; }
    
    /**
     * 获取测试控制器（const版本）
     * @return 测试控制器const引用
     */
    const JTestController& getTestController() const { return testController_; }
    
    /**
     * 创建组件
     * @param type 组件类型
     * @param parent 父组件句柄（可选）
     * @return 新组件句柄
     */
    JComponentHandle createComponent(JComponentType type, JComponentHandle parent = {}) {
        return stateManager_.createComponent(type, parent);
    }
    
    /**
     * 销毁组件
     * @param handle 要销毁的组件句柄
     */
    void destroyComponent(JComponentHandle handle) {
        stateManager_.destroyComponent(handle);
    }
    
    /**
     * 设置组件属性
     * @param h 组件句柄
     * @param id 属性ID
     * @param value 属性值
     */
    void setProperty(JComponentHandle h, JPropertyId id, JPropertyValue value) {
        stateManager_.setProperty(h, id, std::move(value));
    }
    
    /**
     * 获取组件属性
     * @param h 组件句柄
     * @param id 属性ID
     * @return 属性值指针，如果不存在返回nullptr
     */
    const JPropertyValue* getProperty(JComponentHandle h, JPropertyId id) const {
        return stateManager_.getProperty(h, id);
    }
    
    /**
     * 开始批量更新
     */
    void beginBatch() { stateManager_.beginBatch(); }
    
    /**
     * 结束批量更新
     */
    void endBatch() { stateManager_.endBatch(); }
    
    /**
     * 拍摄快照
     * @return 快照JSON字符串
     */
    std::string takeSnapshot() {
        return testController_.takeSnapshot();
    }
    
    /**
     * 设置布局引擎模式
     * @param mode 模式
     */
    void setMode(JLayoutEngineMode mode) {
        layoutEngine_.setMode(mode);
    }
    
    /**
     * 运行一帧
     * 执行必要的布局更新和事件处理
     */
    void runFrame();
    
    /**
     * 分发鼠标移动事件
     * @param x X坐标
     * @param y Y坐标
     */
    void dispatchMouseMove(float x, float y);
    
    /**
     * 分发鼠标按下事件
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void dispatchMouseDown(float x, float y, int button);
    
    /**
     * 分发鼠标释放事件
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void dispatchMouseUp(float x, float y, int button);
    
    /**
     * 分发鼠标点击事件
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void dispatchClick(float x, float y, int button);
    
    /**
     * 分发键盘按下事件
     * @param keyCode 键码
     */
    void dispatchKeyDown(int keyCode);
    
    /**
     * 分发键盘释放事件
     * @param keyCode 键码
     */
    void dispatchKeyUp(int keyCode);
    
    /**
     * 分发文本输入事件
     * @param text 输入文本
     */
    void dispatchTextInput(const std::string& text);
    
    // ========== A2UI JSON界面描述接口 (新增) ==========
    
    /**
     * 从A2UI JSON字符串加载界面
     * 支持v0.9和v0.8两种协议格式
     * @param json A2UI格式的JSON字符串
     * @param surfaceId Surface标识符，默认"main"
     * @return 根组件的A2UI ID，失败返回空字符串
     */
    std::string loadFromA2UI(const std::string& json, const std::string& surfaceId = "main");
    
    /**
     * 将当前界面导出为A2UI JSON字符串
     * @param surfaceId Surface标识符，默认"main"
     * @return A2UI格式的JSON字符串
     */
    std::string exportToA2UI(const std::string& surfaceId = "main") const;
    
    /**
     * 流式添加组件（渐进式渲染）
     * 用于JSONL流式场景，逐条添加组件
     * @param json 单条组件或消息的JSON字符串
     * @param surfaceId Surface标识符，默认"main"
     */
    void streamComponent(const std::string& json, const std::string& surfaceId = "main");
    
    /**
     * 获取A2UI解析器引用（懒初始化）
     * @return A2UI解析器引用
     */
    JA2UIParser& getA2UIParser();
    
    /**
     * 获取A2UI生成器引用（懒初始化）
     * @return A2UI生成器引用
     */
    JA2UIGenerator& getA2UIGenerator();
    
    /**
     * 获取Surface管理器引用（懒初始化）
     * @return Surface管理器引用
     */
    JJSurfaceManager& getSurfaceManager();
    
    /**
     * 获取数据模型引用（懒初始化）
     * @return 数据模型引用
     */
    JJDataModel& getDataModel();
    
private:
    /**
     * 确保A2UI模块已初始化（惰性初始化）
     */
    void ensureA2UIInitialized();
    JComponentStorage storage_;          // 组件存储
    JLayoutEngine layoutEngine_;         // 布局引擎
    JEventDispatcher eventDispatcher_;   // 事件分发器
    JStateManager stateManager_;         // 状态管理器
    JTestController testController_;     // 测试控制器
    
    // A2UI模块（惰性初始化，不影响无A2UI场景的性能）
    std::unique_ptr<JA2UIParser> a2uiParser_;         // A2UI解析器
    std::unique_ptr<JA2UIGenerator> a2uiGenerator_;   // A2UI生成器
    std::unique_ptr<JJSurfaceManager> surfaceManager_; // Surface管理器
    std::unique_ptr<JJDataModel> dataModel_;           // 数据模型
};

}
