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
#include "Direct2DRenderer.h"
#include "StyleSheet.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

namespace jaether {

// ==================== 渲染上下文 ====================

/**
 * 渲染上下文结构体
 * 
 * 封装一次控件渲染所需的全部输入信息
 * 由JRendererFacade在遍历组件树时构建
 */
struct JRenderContext {
    JComponentHandle handle;             // 组件句柄
    JComponentType type;                 // 组件类型枚举
    JRect absoluteRect;                  // 绝对窗口坐标的渲染矩形
    const JPropertyBlock* properties;    // 组件属性块指针（只读）
    bool isHovered = false;              // 鼠标光标是否悬停在此控件上
    bool isFocused = false;              // 此控件是否拥有输入焦点
};

// ==================== 控件渲染器抽象基类 ====================

/**
 * 控件渲染器抽象基类
 * 
 * 策略模式的核心抽象——每个具体控件类型对应一个子类实现
 * 用户可通过继承此类并注册到JControlRendererRegistry来定制任意控件的渲染行为
 */
class JControlRenderer {
public:
    virtual ~JControlRenderer() = default;

    /**
     * 渲染单个控件
     * @param renderer 底层Direct2D渲染器（提供基础绘图能力）
     * @param ctx      渲染上下文（位置、属性、交互状态）
     * @param style    当前生效的样式表
     */
    virtual void render(JDirect2DRenderer* renderer,
                        const JRenderContext& ctx,
                        const JStyleSheet& style) = 0;

    /**
     * 获取此渲染器支持的控件类型标识
     * @return 控件类型枚举值
     */
    virtual JComponentType getComponentType() const = 0;
};

// ==================== 内置控件渲染器 ====================

/**
 * 容器控件渲染器
 * 渲染逻辑：填充柔和灰色背景
 */
class JContainerRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override;
    JComponentType getComponentType() const override;
};

/**
 * 按钮控件渲染器
 * 渲染逻辑：
 *   1. 从属性BackgroundColor读取variant（primary/danger等）决定背景色
 *   2. 根据悬停状态覆盖背景色
 *   3. 绘制圆角矩形背景
 *   4. 从属性读取FontSize决定字号
 *   5. 居中绘制Text属性文本
 */
class JButtonRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override;
    JComponentType getComponentType() const override;
};

/**
 * 文本控件渲染器
 * 渲染逻辑：
 *   1. 从属性读取FontSize（有则用属性值，无则用样式默认值）
 *   2. 左对齐绘制Text属性文本
 */
class JTextRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override;
    JComponentType getComponentType() const override;
};

/**
 * 输入框控件渲染器
 * 渲染逻辑：
 *   1. 填充白色背景
 *   2. 绘制灰色边框
 *   3. 左对齐（带内边距）绘制文本
 */
class JInputRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override;
    JComponentType getComponentType() const override;
};

/**
 * 卡片容器渲染器
 * 渲染逻辑：
 *   1. 绘制阴影偏移矩形
 *   2. 填充白色圆角矩形背景
 *   3. 绘制灰色圆角矩形边框
 */
class JCardRenderer : public JControlRenderer {
public:
    void render(JDirect2DRenderer* renderer, const JRenderContext& ctx,
                const JStyleSheet& style) override;
    JComponentType getComponentType() const override;
};

// ==================== 渲染器注册中心 ====================

/**
 * 控件渲染器注册中心
 * 
 * 维护 JComponentType → JControlRenderer* 的映射表
 * 支持运行时注册、注销、查询渲染器
 * 注意：Registry不接管渲染器对象的所有权，调用方负责生命周期管理
 */
class JControlRendererRegistry {
public:
    JControlRendererRegistry();
    ~JControlRendererRegistry();

    /**
     * 注册/替换指定控件类型的渲染器
     * @param type     控件类型枚举
     * @param renderer 渲染器实例指针（Registry不接管所有权）
     */
    void registerRenderer(JComponentType type, JControlRenderer* renderer);

    /**
     * 注销指定控件类型的渲染器
     * @param type 控件类型枚举
     */
    void unregisterRenderer(JComponentType type);

    /**
     * 获取指定控件类型的渲染器
     * @param type 控件类型枚举
     * @return 渲染器指针，未注册返回nullptr
     */
    JControlRenderer* getRenderer(JComponentType type) const;

    /**
     * 检查是否已注册指定控件类型的渲染器
     * @param type 控件类型枚举
     * @return 已注册返回true
     */
    bool hasRenderer(JComponentType type) const;

    /**
     * 获取所有已注册的控件类型列表
     * @return 控件类型枚举列表
     */
    std::vector<JComponentType> getRegisteredTypes() const;

    /**
     * 注册所有内置渲染器（Container/Button/Text/Input/Card）
     * 使用静态实例，生命周期与进程一致
     */
    static void registerDefaults(JControlRendererRegistry& registry);

private:
    // JComponentType → JControlRenderer* 映射表
    std::unordered_map<JComponentType, JControlRenderer*> map_;
};

// ==================== 统一渲染门面 ====================

/**
 * 统一渲染门面类
 * 
 * 应用层通过此门面完成所有控件渲染工作，无需关心内部渲染器细节
 * 职责：遍历组件树 → 构建JRenderContext → 查找Renderer → 调用render()
 * 
 * 典型用法：
 *   JRendererFacade facade(&renderer, storage);
 *   facade.renderAll(mouseX, mouseY);
 */
class JRendererFacade {
public:
    /**
     * 构造函数
     * @param renderer 底层Direct2D渲染器（必须已初始化）
     * @param storage  组件存储引用
     */
    JRendererFacade(JDirect2DRenderer* renderer, JComponentStorage& storage);

    /**
     * 渲染所有可见组件
     * 遍历组件树，为每个可见组件构建渲染上下文并调用对应渲染器
     * @param mouseX 当前鼠标X坐标（DIP单位，-1表示不检测悬停）
     * @param mouseY 当前鼠标Y坐标（DIP单位，-1表示不检测悬停）
     */
    void renderAll(float mouseX = -1.0f, float mouseY = -1.0f);

    /**
     * 渲染单个指定组件
     * @param handle 目标组件句柄
     * @param mouseX 鼠标X坐标（-1表示不检测悬停）
     * @param mouseY 鼠标Y坐标（-1表示不检测悬停）
     */
    void renderOne(JComponentHandle handle, float mouseX = -1.0f, float mouseY = -1.0f);

    /**
     * 获取渲染器注册中心（用于注册/替换自定义渲染器）
     * @return 注册中心引用
     */
    JControlRendererRegistry& getRegistry() { return registry_; }

    /**
     * 获取当前样式表（可读写，用于动态调整样式）
     * @return 样式表引用
     */
    JStyleSheet& getStyleSheet() { return styleSheet_; }
    const JStyleSheet& getStyleSheet() const { return styleSheet_; }

    /**
     * 整体替换样式表（用于主题切换）
     * @param sheet 新的样式表
     */
    void setStyleSheet(const JStyleSheet& sheet) { styleSheet_ = sheet; }

    /**
     * 清空画布（使用当前样式表的clearColor）
     */
    void clearCanvas();

    /**
     * 获取底层渲染器（只读）
     */
    JDirect2DRenderer* getRenderer() { return renderer_; }

private:
    /**
     * 构建单个组件的渲染上下文
     * @param handle 组件句柄
     * @param mouseX 鼠标X坐标
     * @param mouseY 鼠标Y坐标
     * @return 填充好的渲染上下文
     */
    JRenderContext buildContext(JComponentHandle handle, float mouseX, float mouseY);

    JDirect2DRenderer* renderer_;       // 底层Direct2D渲染器
    JComponentStorage& storage_;         // 组件存储引用
    JControlRendererRegistry registry_;  // 渲染器注册中心
    JStyleSheet styleSheet_;             // 当前样式表
};

} // namespace jaether
