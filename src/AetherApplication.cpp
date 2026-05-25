// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// AetherApplication.cpp
// Aether应用程序基础框架 — 封装窗口创建、消息循环、渲染循环
//
// 不包含任何测试/样例代码。应用层通过getLogicLayer()等访问器创建UI，
// 通过setFrameCallback()注册每帧业务逻辑。

#include "aether/AetherApplication.h"
#include "aether/ControlRenderer.h"
#include <iostream>
#include <windowsx.h>

namespace jaether {

JAetherApplication::JAetherApplication() {
}

JAetherApplication::~JAetherApplication() {
    shutdown();
}

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
    
    // 创建窗口（默认800×600，应用层可在initialize后通过onResize调整）
    hwnd_ = CreateWindowExW(
        0, L"AetherWindow", L"Aether UI Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 700,
        NULL, NULL, hInstance, this);
    
    if (!hwnd_) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    
    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    // 创建逻辑层和渲染器（不创建测试UI）
    logicLayer_ = std::make_unique<JLogicLayer>();
    renderer_ = std::make_unique<JDirect2DRenderer>();
    
    if (!renderer_->initialize(hwnd_)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
    
    // 60fps定时器
    SetTimer(hwnd_, 1, 16, NULL);
    
    running_ = true;
    return true;
}

void JAetherApplication::shutdown() {
    if (!running_) return;
    running_ = false;
    
    KillTimer(hwnd_, 1);
    
    if (renderer_) {
        renderer_->shutdown();
        renderer_.reset();
    }
    renderFacade_.reset();
    logicLayer_.reset();
    
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void JAetherApplication::run() {
    MSG msg = {0};
    while (running_ && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// ========== 窗口过程 ==========

LRESULT CALLBACK JAetherApplication::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    JAetherApplication* app = nullptr;
    
    if (uMsg == WM_CREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<JAetherApplication*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<JAetherApplication*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    
    if (!app) return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    
    switch (uMsg) {
        case WM_TIMER:   { app->update(); return 0; }
        case WM_PAINT:   { app->onPaint(); return 0; }
        case WM_SIZE:    { app->onResize(LOWORD(lParam), HIWORD(lParam)); return 0; }
        case WM_MOUSEMOVE: { app->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); return 0; }
        case WM_LBUTTONDOWN: { app->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0); return 0; }
        case WM_LBUTTONUP: {
            int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
            app->onMouseUp(x, y, 0);
            app->dispatchClick(x, y, 0);
            return 0;
        }
        case WM_KEYDOWN: { app->onKeyDown(static_cast<int>(wParam)); return 0; }
        case WM_KEYUP:   { app->onKeyUp(static_cast<int>(wParam)); return 0; }
        case WM_CHAR:    { app->onChar(static_cast<char>(wParam)); return 0; }
        case WM_DESTROY: { PostQuitMessage(0); return 0; }
        default: return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

// ========== 输入事件处理 ==========

void JAetherApplication::onPaint() {
    PAINTSTRUCT ps;
    BeginPaint(hwnd_, &ps);
    render();
    EndPaint(hwnd_, &ps);
}

void JAetherApplication::onResize(int width, int height) {
    if (renderer_) renderer_->resize(width, height);
    InvalidateRect(hwnd_, NULL, FALSE);
}

void JAetherApplication::onMouseMove(int x, int y) {
    if (logicLayer_ && renderer_) {
        float sx = 96.0f / renderer_->getDpiX();
        float sy = 96.0f / renderer_->getDpiY();
        lastMouseX_ = static_cast<float>(x) * sx;
        lastMouseY_ = static_cast<float>(y) * sy;
        logicLayer_->dispatchMouseMove(lastMouseX_, lastMouseY_);
    }
}

void JAetherApplication::onMouseDown(int x, int y, int button) {
    if (logicLayer_ && renderer_) {
        float sx = 96.0f / renderer_->getDpiX();
        float sy = 96.0f / renderer_->getDpiY();
        logicLayer_->dispatchMouseDown(static_cast<float>(x) * sx, static_cast<float>(y) * sy, button);
    }
}

void JAetherApplication::onMouseUp(int x, int y, int button) {
    if (logicLayer_ && renderer_) {
        float sx = 96.0f / renderer_->getDpiX();
        float sy = 96.0f / renderer_->getDpiY();
        logicLayer_->dispatchMouseUp(static_cast<float>(x) * sx, static_cast<float>(y) * sy, button);
    }
}

void JAetherApplication::dispatchClick(int x, int y, int button) {
    if (logicLayer_ && renderer_) {
        float sx = 96.0f / renderer_->getDpiX();
        float sy = 96.0f / renderer_->getDpiY();
        logicLayer_->dispatchClick(static_cast<float>(x) * sx, static_cast<float>(y) * sy, button);
    }
}

void JAetherApplication::onKeyDown(int keyCode) {
    if (logicLayer_) logicLayer_->dispatchKeyDown(keyCode);
}

void JAetherApplication::onKeyUp(int keyCode) {
    if (logicLayer_) logicLayer_->dispatchKeyUp(keyCode);
}

void JAetherApplication::onChar(char ch) {
    if (logicLayer_) logicLayer_->dispatchTextInput(std::string(1, ch));
}

// ========== 渲染与更新循环 ==========

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

void JAetherApplication::update() {
    if (logicLayer_) {
        logicLayer_->runFrame();
        
        // 调用应用层注册的帧回调
        if (frameCallback_) {
            frameCallback_(*logicLayer_);
        }
    }
    
    InvalidateRect(hwnd_, NULL, FALSE);
}

void JAetherApplication::requestRedraw() {
    InvalidateRect(hwnd_, NULL, FALSE);
}

} // namespace jaether
