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

#include "aether/LogicLayer.h"
#include "aether/Direct2DRenderer.h"
#include "aether/IPCClient.h"
#include "aether/JSONParser.h"
#include <memory>
#include <functional>

namespace jaether {

// 前向声明
class JRendererFacade;

/**
 * Aether应用程序基础类
 * 
 * 封装窗口创建、消息循环、渲染循环等通用基础设施
 * 不包含任何测试/样例代码，保持框架纯净
 * 应用层通过getLogicLayer()等访问器在外部创建UI
 */
class JAetherApplication {
public:
    /**
     * 帧回调函数类型
     * 每帧update时被调用，参数为逻辑层引用，应用层在此处理业务逻辑
     */
    using FrameCallback = std::function<void(JLogicLayer&)>;

    /**
     * 构造函数
     */
    JAetherApplication();
    
    /**
     * 析构函数
     */
    ~JAetherApplication();
    
    /**
     * 初始化应用程序（创建窗口、逻辑层、渲染器）
     * @param hInstance 应用程序实例句柄
     * @param nCmdShow 显示命令
     * @return 是否初始化成功
     */
    bool initialize(HINSTANCE hInstance, int nCmdShow);
    
    /**
     * 关闭应用程序，清理所有资源
     */
    void shutdown();
    
    /**
     * 运行消息循环（阻塞直到窗口关闭）
     */
    void run();
    
    /**
     * 设置每帧回调
     * 在每帧update时被调用，应用层在此处理自定义逻辑（如点击事件分发）
     * @param callback 回调函数，参数为JLogicLayer引用
     */
    void setFrameCallback(FrameCallback callback) { frameCallback_ = std::move(callback); }
    
    /**
     * 获取逻辑层指针（应用层通过此访问器创建UI组件）
     * @return 逻辑层指针，未初始化返回nullptr
     */
    JLogicLayer* getLogicLayer() { return logicLayer_.get(); }
    
    /**
     * 获取Direct2D渲染器指针
     * @return 渲染器指针，未初始化返回nullptr
     */
    JDirect2DRenderer* getRenderer() { return renderer_.get(); }
    
    /**
     * 获取控件渲染门面指针
     * @return 门面指针，未初始化返回nullptr
     */
    JRendererFacade* getRenderFacade() { return renderFacade_.get(); }
    
    /**
     * 获取窗口句柄
     * @return 窗口句柄
     */
    HWND getHwnd() const { return hwnd_; }
    
    // ========== 消息处理（由WindowProc调用，public用于外部触发重绘） ==========
    
    void onPaint();
    void onResize(int width, int height);
    void onMouseMove(int x, int y);
    void onMouseDown(int x, int y, int button);
    void onMouseUp(int x, int y, int button);
    void dispatchClick(int x, int y, int button);
    void onKeyDown(int keyCode);
    void onKeyUp(int keyCode);
    void onChar(char ch);
    
    /** 触发重绘（应用层在修改UI后调用） */
    void requestRedraw();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void render();
    void update();
    
    std::unique_ptr<JLogicLayer> logicLayer_;
    std::unique_ptr<JDirect2DRenderer> renderer_;
    std::unique_ptr<JRendererFacade> renderFacade_;
    JJSONValue stateNode_;
    
    HWND hwnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;
    bool running_ = false;
    float lastMouseX_ = -1.0f;
    float lastMouseY_ = -1.0f;
    
    FrameCallback frameCallback_;    // 每帧回调，由应用层设置
};

} // namespace jaether
