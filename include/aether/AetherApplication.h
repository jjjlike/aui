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
 * Aether应用程序类
 * 
 * 整合所有模块的高层应用类
 * 提供完整的UI应用程序功能
 */
class JAetherApplication {
public:
    /**
     * 构造函数
     */
    JAetherApplication();
    
    /**
     * 析构函数
     */
    ~JAetherApplication();
    
    /**
     * 初始化应用程序
     * @param hInstance 应用程序实例句柄
     * @param nCmdShow 显示命令
     * @return 是否初始化成功
     */
    bool initialize(HINSTANCE hInstance, int nCmdShow);
    
    /**
     * 关闭应用程序
     * 清理所有资源
     */
    void shutdown();
    
    /**
     * 运行应用程序主循环
     */
    void run();
    
    /**
     * 处理绘制消息
     */
    void onPaint();
    
    /**
     * 处理窗口大小调整
     * @param width 新宽度
     * @param height 新高度
     */
    void onResize(int width, int height);
    
    /**
     * 处理鼠标移动
     * @param x X坐标
     * @param y Y坐标
     */
    void onMouseMove(int x, int y);
    
    /**
     * 处理鼠标按下
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void onMouseDown(int x, int y, int button);
    
    /**
     * 处理鼠标释放
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void onMouseUp(int x, int y, int button);
    
    /**
     * 处理鼠标点击
     * @param x X坐标
     * @param y Y坐标
     * @param button 鼠标按钮
     */
    void dispatchClick(int x, int y, int button);
    
    /**
     * 处理键盘按下
     * @param keyCode 键码
     */
    void onKeyDown(int keyCode);
    
    /**
     * 处理键盘释放
     * @param keyCode 键码
     */
    void onKeyUp(int keyCode);
    
    /**
     * 处理字符输入
     * @param ch 字符
     */
    void onChar(char ch);
    
    /**
     * 获取窗口句柄
     * @return 窗口句柄
     */
    HWND getHwnd() const { return hwnd_; }
    
private:
    /**
     * 窗口过程回调函数
     * @param hwnd 窗口句柄
     * @param uMsg 消息ID
     * @param wParam 消息参数
     * @param lParam 消息参数
     * @return 处理结果
     */
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    /**
     * 创建测试UI
     */
    void createTestUI();
    
    /**
     * 渲染一帧
     */
    void render();
    
    /**
     * 更新一帧
     */
    void update();
    
    std::unique_ptr<JLogicLayer> logicLayer_;          // 逻辑层
    std::unique_ptr<JDirect2DRenderer> renderer_;       // 渲染器
    std::unique_ptr<JRendererFacade> renderFacade_;     // 控件渲染门面（惰性初始化）
    JJSONValue stateNode_;                              // 状态节点
    
    HWND hwnd_ = nullptr;       // 窗口句柄
    HINSTANCE hInstance_ = nullptr;  // 应用实例句柄
    bool running_ = false;     // 运行状态
    float lastMouseX_ = -1.0f;  // 最后一次鼠标X坐标
    float lastMouseY_ = -1.0f;  // 最后一次鼠标Y坐标
};

}
