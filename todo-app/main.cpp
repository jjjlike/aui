// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// todo-app/main.cpp
// 待办列表应用程序入口
//
// 功能: 完整的待办列表应用
// 作者: Aether UI Engine Demo App

#include "TodoApp.h"
#include <aether/Direct2DRenderer.h>
#include <aether/ControlRenderer.h>
#include <aether/Logger.h>
#include <windows.h>
#include <windowsx.h>
#include <memory>
#include <iostream>

// 前向声明
class TodoAppWindow;

// 全局窗口类实现
class TodoAppWindow {
private:
    HWND hwnd_ = nullptr;
    std::unique_ptr<jaether::JLogicLayer> logicLayer_;
    std::unique_ptr<jaether::JTodoApp> todoApp_;
    std::unique_ptr<jaether::JDirect2DRenderer> renderer_;
    std::unique_ptr<jaether::JRendererFacade> renderFacade_;  // 控件渲染门面
    bool running_ = false;
    float mouseX_ = -1.0f;  // 当前鼠标X坐标（DIP）
    float mouseY_ = -1.0f;  // 当前鼠标Y坐标（DIP）

    // 窗口过程函数（静态回调）
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        TodoAppWindow* app = nullptr;
        
        if (uMsg == WM_CREATE) {
            // 获取应用程序实例
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            app = reinterpret_cast<TodoAppWindow*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        } else {
            app = reinterpret_cast<TodoAppWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (!app) {
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }

        switch (uMsg) {
            case WM_TIMER:
                app->update();
                return 0;
                
            case WM_PAINT:
                app->render();
                return 0;
                
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
                app->onClick(x, y, 0);
                return 0;
            }
                
            case WM_KEYDOWN:
                app->onKeyDown(static_cast<int>(wParam));
                return 0;
                
            case WM_KEYUP:
                app->onKeyUp(static_cast<int>(wParam));
                return 0;
                
            case WM_CHAR:
                app->onChar(static_cast<char>(wParam));
                return 0;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
                
            default:
                return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
    }

public:
    TodoAppWindow() = default;
    ~TodoAppWindow() {
        shutdown();
    }

    bool initialize(HINSTANCE hInstance, int nCmdShow) {
        // 注册窗口类
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"TodoAppWindow";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.style = CS_HREDRAW | CS_VREDRAW;

        if (!RegisterClassW(&wc)) {
            MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_ICONERROR);
            return false;
        }

        // 创建窗口
        hwnd_ = CreateWindowExW(
            0,
            L"TodoAppWindow",
            L"待办列表",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            800, 600,
            nullptr,
            nullptr,
            hInstance,
            this
        );

        if (!hwnd_) {
            MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_ICONERROR);
            return false;
        }

        // 初始化逻辑层和待办应用
        logicLayer_ = std::make_unique<jaether::JLogicLayer>();
        todoApp_ = std::make_unique<jaether::JTodoApp>(*logicLayer_);
        todoApp_->initialize();

        // 初始化渲染器
        renderer_ = std::make_unique<jaether::JDirect2DRenderer>();
        if (!renderer_->initialize(hwnd_)) {
            MessageBoxW(nullptr, L"Failed to initialize renderer", L"Error", MB_ICONERROR);
            return false;
        }

        // 显示窗口
        ShowWindow(hwnd_, nCmdShow);
        UpdateWindow(hwnd_);

        // 获取窗口实际大小并更新根容器
        RECT clientRect;
        GetClientRect(hwnd_, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        
        float dpiScaleX = 96.0f / renderer_->getDpiX();
        float dpiScaleY = 96.0f / renderer_->getDpiY();
        float dipWidth = static_cast<float>(width) * dpiScaleX;
        float dipHeight = static_cast<float>(height) * dpiScaleY;
        
        jaether::JLogger::getInstance().info("窗口客户区大小: " + std::to_string(width) + "x" + std::to_string(height) +
            " DPI: " + std::to_string(static_cast<int>(renderer_->getDpiX())));
        jaether::JLogger::getInstance().info("DIP尺寸: " + std::to_string(static_cast<int>(dipWidth)) + "x" + std::to_string(static_cast<int>(dipHeight)));
        jaether::JLogger::getInstance().info("更新根容器尺寸为: " + std::to_string(dipWidth) + "x" + std::to_string(dipHeight));
        logicLayer_->setProperty(todoApp_->getRootContainer(), jaether::JPropertyId::Width, jaether::JPropertyValue(dipWidth));
        logicLayer_->setProperty(todoApp_->getRootContainer(), jaether::JPropertyId::Height, jaether::JPropertyValue(dipHeight));
        jaether::JLogger::getInstance().info("调用runFrame()计算布局");
        logicLayer_->runFrame();
        jaether::JLogger::getInstance().info("布局计算完成");

        // 设置定时器
        SetTimer(hwnd_, 1, 16, nullptr); // 约60fps

        running_ = true;
        return true;
    }

    void shutdown() {
        if (!running_) return;

        running_ = false;

        // 停止定时器
        if (hwnd_) {
            KillTimer(hwnd_, 1);
        }

        // 清理资源
        if (renderer_) {
            renderer_->shutdown();
            renderer_.reset();
        }

        todoApp_.reset();
        logicLayer_.reset();

        if (hwnd_) {
            DestroyWindow(hwnd_);
            hwnd_ = nullptr;
        }
    }

    void run() {
        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void onResize(int width, int height) {
        if (renderer_) {
            renderer_->resize(width, height);
        }
        
        if (logicLayer_ && todoApp_ && renderer_) {
            float dpiScaleX = 96.0f / renderer_->getDpiX();
            float dpiScaleY = 96.0f / renderer_->getDpiY();
            logicLayer_->setProperty(todoApp_->getRootContainer(), jaether::JPropertyId::Width, jaether::JPropertyValue(static_cast<float>(width) * dpiScaleX));
            logicLayer_->setProperty(todoApp_->getRootContainer(), jaether::JPropertyId::Height, jaether::JPropertyValue(static_cast<float>(height) * dpiScaleY));
        }
    }

    void onMouseMove(int x, int y) {
        if (logicLayer_ && renderer_) {
            float dpiScaleX = 96.0f / renderer_->getDpiX();
            float dpiScaleY = 96.0f / renderer_->getDpiY();
            mouseX_ = static_cast<float>(x) * dpiScaleX;
            mouseY_ = static_cast<float>(y) * dpiScaleY;
            logicLayer_->dispatchMouseMove(mouseX_, mouseY_);
        }
    }

    void onMouseDown(int x, int y, int button) {
        if (logicLayer_ && renderer_) {
            float dpiScaleX = 96.0f / renderer_->getDpiX();
            float dpiScaleY = 96.0f / renderer_->getDpiY();
            logicLayer_->dispatchMouseDown(static_cast<float>(x) * dpiScaleX, static_cast<float>(y) * dpiScaleY, button);
        }
    }

    void onMouseUp(int x, int y, int button) {
        if (logicLayer_ && renderer_) {
            float dpiScaleX = 96.0f / renderer_->getDpiX();
            float dpiScaleY = 96.0f / renderer_->getDpiY();
            logicLayer_->dispatchMouseUp(static_cast<float>(x) * dpiScaleX, static_cast<float>(y) * dpiScaleY, button);
        }
    }

    void onClick(int x, int y, int button) {
        if (logicLayer_ && renderer_) {
            float dpiScaleX = 96.0f / renderer_->getDpiX();
            float dpiScaleY = 96.0f / renderer_->getDpiY();
            logicLayer_->dispatchClick(static_cast<float>(x) * dpiScaleX, static_cast<float>(y) * dpiScaleY, button);
        }
    }

    void onKeyDown(int keyCode) {
        if (logicLayer_) {
            logicLayer_->dispatchKeyDown(keyCode);
        }
    }

    void onKeyUp(int keyCode) {
        if (logicLayer_) {
            logicLayer_->dispatchKeyUp(keyCode);
        }
    }

    void onChar(char ch) {
        if (logicLayer_) {
            logicLayer_->dispatchTextInput(std::string(1, ch));
        }
    }

    void update() {
        if (logicLayer_) {
            logicLayer_->runFrame();
        }

        if (todoApp_) {
            todoApp_->update();
        }

        if (hwnd_) {
            InvalidateRect(hwnd_, nullptr, FALSE);
        }
    }

    void render() {
        PAINTSTRUCT ps;
        BeginPaint(hwnd_, &ps);

        if (renderer_ && logicLayer_) {
            // 惰性初始化渲染门面
            if (!renderFacade_) {
                renderFacade_ = std::make_unique<jaether::JRendererFacade>(
                    renderer_.get(), logicLayer_->getStorage());
            }
            
            renderer_->beginDraw();
            renderFacade_->clearCanvas();
            renderFacade_->renderAll(mouseX_, mouseY_);
            renderer_->endDraw();
        }

        EndPaint(hwnd_, &ps);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 初始化日志
    jaether::JLogger::getInstance().setLevel(jaether::JLogLevel::Debug);
    jaether::JLogger::getInstance().enableFileOutput(true, "todoapp_debug.log");
    jaether::JLogger::getInstance().info("=== JTodoApp 启动 ===");
    
    // 初始化COM库（Direct2D和WIC需要）
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to initialize COM library", L"Error", MB_ICONERROR);
        return 1;
    }

    // 防止闪烁
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    TodoAppWindow app;

    if (!app.initialize(hInstance, nCmdShow)) {
        return 1;
    }

    jaether::JLogger::getInstance().info("开始消息循环...");
    app.run();
    jaether::JLogger::getInstance().info("=== JTodoApp 退出 ===");

    return 0;
}
