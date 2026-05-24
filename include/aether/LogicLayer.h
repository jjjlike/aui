#pragma once

#include "types.h"
#include "ComponentStorage.h"
#include "LayoutEngine.h"
#include "EventDispatcher.h"
#include "StateManager.h"
#include "Snapshot.h"
#include "TestController.h"

namespace aether {

/**
 * 逻辑层类
 * 
 * 整合所有核心组件的高层接口
 * 提供统一的访问点，简化使用
 * 是框架的核心协调者
 */
class LogicLayer {
public:
    /**
     * 构造函数
     */
    LogicLayer();
    
    /**
     * 析构函数
     */
    ~LogicLayer();
    
    /**
     * 获取组件存储（可变版本）
     * @return 组件存储引用
     */
    ComponentStorage& getStorage() { return storage_; }
    
    /**
     * 获取组件存储（const版本）
     * @return 组件存储const引用
     */
    const ComponentStorage& getStorage() const { return storage_; }
    
    /**
     * 获取布局引擎（可变版本）
     * @return 布局引擎引用
     */
    LayoutEngine& getLayoutEngine() { return layoutEngine_; }
    
    /**
     * 获取布局引擎（const版本）
     * @return 布局引擎const引用
     */
    const LayoutEngine& getLayoutEngine() const { return layoutEngine_; }
    
    /**
     * 获取事件分发器（可变版本）
     * @return 事件分发器引用
     */
    EventDispatcher& getEventDispatcher() { return eventDispatcher_; }
    
    /**
     * 获取事件分发器（const版本）
     * @return 事件分发器const引用
     */
    const EventDispatcher& getEventDispatcher() const { return eventDispatcher_; }
    
    /**
     * 获取状态管理器（可变版本）
     * @return 状态管理器引用
     */
    StateManager& getStateManager() { return stateManager_; }
    
    /**
     * 获取状态管理器（const版本）
     * @return 状态管理器const引用
     */
    const StateManager& getStateManager() const { return stateManager_; }
    
    /**
     * 获取测试控制器（可变版本）
     * @return 测试控制器引用
     */
    TestController& getTestController() { return testController_; }
    
    /**
     * 获取测试控制器（const版本）
     * @return 测试控制器const引用
     */
    const TestController& getTestController() const { return testController_; }
    
    /**
     * 创建组件
     * @param type 组件类型
     * @param parent 父组件句柄（可选）
     * @return 新组件句柄
     */
    ComponentHandle createComponent(ComponentType type, ComponentHandle parent = {}) {
        return stateManager_.createComponent(type, parent);
    }
    
    /**
     * 销毁组件
     * @param handle 要销毁的组件句柄
     */
    void destroyComponent(ComponentHandle handle) {
        stateManager_.destroyComponent(handle);
    }
    
    /**
     * 设置组件属性
     * @param h 组件句柄
     * @param id 属性ID
     * @param value 属性值
     */
    void setProperty(ComponentHandle h, PropertyId id, PropertyValue value) {
        stateManager_.setProperty(h, id, std::move(value));
    }
    
    /**
     * 获取组件属性
     * @param h 组件句柄
     * @param id 属性ID
     * @return 属性值指针，如果不存在返回nullptr
     */
    const PropertyValue* getProperty(ComponentHandle h, PropertyId id) const {
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
    void setMode(LayoutEngineMode mode) {
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
    
private:
    ComponentStorage storage_;          // 组件存储
    LayoutEngine layoutEngine_;         // 布局引擎
    EventDispatcher eventDispatcher_;   // 事件分发器
    StateManager stateManager_;         // 状态管理器
    TestController testController_;     // 测试控制器
};

}
