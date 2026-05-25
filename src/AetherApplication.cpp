// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// JAetherApplication.cpp
// Aether应用程序主模块 - Windows GUI应用程序
//
// 功能:
// - 窗口创建和管理
// - 消息循环处理
// - UI组件渲染
// - 事件分发
// - 测试UI创建

#include "aether/AetherApplication.h"
#include "aether/ControlRenderer.h"
#include <iostream>
#include <windowsx.h>

namespace jaether {

// Aether应用程序构造函数
JAetherApplication::JAetherApplication() {
}

// Aether应用程序析构函数
JAetherApplication::~JAetherApplication() {
    shutdown();
}

// 初始化应用程序
// 参数:
//   hInstance - 应用程序实例句柄
//   nCmdShow - 窗口显示方式
// 返回值: 成功返回true
bool JAetherApplication::initialize(HINSTANCE hInstance, int nCmdShow) {
    hInstance_ = hInstance;
    
    // 注册窗口类
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AetherWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassW(&wc);
    
    // 创建窗口
    hwnd_ = CreateWindowExW(
        0,
        L"AetherWindow",
        L"Aether UI Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL, NULL,
        hInstance,
        this
    );
    
    if (!hwnd_) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    
    // 设置用户数据指针
    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    // 创建逻辑层和渲染器
    logicLayer_ = std::make_unique<JLogicLayer>();
    renderer_ = std::make_unique<JDirect2DRenderer>();
    
    if (!renderer_->initialize(hwnd_)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    // 创建测试UI
    createTestUI();
    
    // 显示窗口
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
    
    // 设置定时器（用于更新）
    SetTimer(hwnd_, 1, 16, NULL);
    
    running_ = true;
    return true;
}

// 关闭应用程序
void JAetherApplication::shutdown() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // 销毁定时器
    KillTimer(hwnd_, 1);
    
    // 关闭渲染器
    if (renderer_) {
        renderer_->shutdown();
        renderer_.reset();
    }
    
    // 释放逻辑层
    logicLayer_.reset();
    
    // 销毁窗口
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

// 运行消息循环
void JAetherApplication::run() {
    MSG msg = {0};
    while (running_ && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// 窗口过程函数（静态）
// 参数:
//   hwnd - 窗口句柄
//   uMsg - 消息类型
//   wParam, lParam - 消息参数
// 返回值: 消息处理结果
LRESULT CALLBACK JAetherApplication::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    JAetherApplication* app = nullptr;
    
    if (uMsg == WM_CREATE) {
        // 从创建参数获取应用程序指针
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<JAetherApplication*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        // 从用户数据获取应用程序指针
        app = reinterpret_cast<JAetherApplication*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    
    if (!app) {
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    
    // 处理消息
    switch (uMsg) {
        case WM_TIMER: {
            app->update();
            return 0;
        }
        
        case WM_PAINT: {
            app->onPaint();
            return 0;
        }
        
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            app->onResize(width, height);
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            app->onMouseMove(x, y);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            app->onMouseDown(x, y, 0);
            return 0;
        }
        
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            app->onMouseUp(x, y, 0);
            app->dispatchClick(x, y, 0);
            return 0;
        }
        
        case WM_KEYDOWN: {
            app->onKeyDown(static_cast<int>(wParam));
            return 0;
        }
        
        case WM_KEYUP: {
            app->onKeyUp(static_cast<int>(wParam));
            return 0;
        }
        
        case WM_CHAR: {
            app->onChar(static_cast<char>(wParam));
            return 0;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

// 创建测试UI
void JAetherApplication::createTestUI() {
    if (!logicLayer_) return;
    
    // 创建根容器
    auto root = logicLayer_->createComponent(JComponentType::Container);
    logicLayer_->setProperty(root, JPropertyId::Width, JPropertyValue(800.0f));
    logicLayer_->setProperty(root, JPropertyId::Height, JPropertyValue(600.0f));
    
    // 创建按钮1
    auto btn1 = logicLayer_->createComponent(JComponentType::Button, root);
    logicLayer_->setProperty(btn1, JPropertyId::X, JPropertyValue(100.0f));
    logicLayer_->setProperty(btn1, JPropertyId::Y, JPropertyValue(100.0f));
    logicLayer_->setProperty(btn1, JPropertyId::Width, JPropertyValue(200.0f));
    logicLayer_->setProperty(btn1, JPropertyId::Height, JPropertyValue(50.0f));
    logicLayer_->setProperty(btn1, JPropertyId::Text, JPropertyValue(std::string("Click Me!")));
    
    // 创建按钮2
    auto btn2 = logicLayer_->createComponent(JComponentType::Button, root);
    logicLayer_->setProperty(btn2, JPropertyId::X, JPropertyValue(100.0f));
    logicLayer_->setProperty(btn2, JPropertyId::Y, JPropertyValue(200.0f));
    logicLayer_->setProperty(btn2, JPropertyId::Width, JPropertyValue(200.0f));
    logicLayer_->setProperty(btn2, JPropertyId::Height, JPropertyValue(50.0f));
    logicLayer_->setProperty(btn2, JPropertyId::Text, JPropertyValue(std::string("Submit")));
    
    // 创建文本组件
    auto txt = logicLayer_->createComponent(JComponentType::Text, root);
    logicLayer_->setProperty(txt, JPropertyId::X, JPropertyValue(100.0f));
    logicLayer_->setProperty(txt, JPropertyId::Y, JPropertyValue(300.0f));
    logicLayer_->setProperty(txt, JPropertyId::Width, JPropertyValue(600.0f));
    logicLayer_->setProperty(txt, JPropertyId::Height, JPropertyValue(100.0f));
    logicLayer_->setProperty(txt, JPropertyId::Text, JPropertyValue(std::string("Welcome to Aether UI Engine!")));
    
    // 运行一帧更新
    logicLayer_->runFrame();
}

// 绘制回调
void JAetherApplication::onPaint() {
    PAINTSTRUCT ps;
    BeginPaint(hwnd_, &ps);
    
    render();
    
    EndPaint(hwnd_, &ps);
}

// 窗口大小改变回调
// 参数: width, height - 新尺寸
void JAetherApplication::onResize(int width, int height) {
    if (renderer_) {
        renderer_->resize(width, height);
    }
    InvalidateRect(hwnd_, NULL, FALSE);
}

// 鼠标移动回调
// 参数: x, y - 鼠标坐标
void JAetherApplication::onMouseMove(int x, int y) {
    if (logicLayer_ && renderer_) {
        float dpiScaleX = 96.0f / renderer_->getDpiX();
        float dpiScaleY = 96.0f / renderer_->getDpiY();
        float dipX = static_cast<float>(x) * dpiScaleX;
        float dipY = static_cast<float>(y) * dpiScaleY;
        lastMouseX_ = dipX;
        lastMouseY_ = dipY;
        logicLayer_->dispatchMouseMove(dipX, dipY);
    }
}

// 鼠标按下回调
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JAetherApplication::onMouseDown(int x, int y, int button) {
    if (logicLayer_ && renderer_) {
        float dpiScaleX = 96.0f / renderer_->getDpiX();
        float dpiScaleY = 96.0f / renderer_->getDpiY();
        logicLayer_->dispatchMouseDown(static_cast<float>(x) * dpiScaleX, static_cast<float>(y) * dpiScaleY, button);
    }
}

// 鼠标释放回调
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JAetherApplication::onMouseUp(int x, int y, int button) {
    if (logicLayer_ && renderer_) {
        float dpiScaleX = 96.0f / renderer_->getDpiX();
        float dpiScaleY = 96.0f / renderer_->getDpiY();
        logicLayer_->dispatchMouseUp(static_cast<float>(x) * dpiScaleX, static_cast<float>(y) * dpiScaleY, button);
    }
}

// 分发点击事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void JAetherApplication::dispatchClick(int x, int y, int button) {
    if (logicLayer_ && renderer_) {
        float dpiScaleX = 96.0f / renderer_->getDpiX();
        float dpiScaleY = 96.0f / renderer_->getDpiY();
        logicLayer_->dispatchClick(static_cast<float>(x) * dpiScaleX, static_cast<float>(y) * dpiScaleY, button);
    }
}

// 按键按下回调
// 参数: keyCode - 按键码
void JAetherApplication::onKeyDown(int keyCode) {
    if (logicLayer_) {
        logicLayer_->dispatchKeyDown(keyCode);
    }
}

// 按键释放回调
// 参数: keyCode - 按键码
void JAetherApplication::onKeyUp(int keyCode) {
    if (logicLayer_) {
        logicLayer_->dispatchKeyUp(keyCode);
    }
}

// 字符输入回调
// 参数: ch - 输入的字符
void JAetherApplication::onChar(char ch) {
    if (logicLayer_) {
        logicLayer_->dispatchTextInput(std::string(1, ch));
    }
}

// 渲染UI — 使用统一的控件渲染器体系替代原先分散的switch/case
void JAetherApplication::render() {
    if (!renderer_ || !logicLayer_) return;
    
    // 惰性初始化渲染门面
    if (!renderFacade_) {
        renderFacade_ = std::make_unique<JRendererFacade>(
            renderer_.get(), logicLayer_->getStorage());
    }
    
    renderer_->beginDraw();
    renderFacade_->clearCanvas();
    renderFacade_->renderAll(lastMouseX_, lastMouseY_);
    renderer_->endDraw();
}

// 更新一帧
void JAetherApplication::update() {
    if (logicLayer_) {
        logicLayer_->runFrame();
    }
    
    InvalidateRect(hwnd_, NULL, FALSE);
}

} // namespace jaether
