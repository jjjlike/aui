// AetherApplication.cpp
// Aether应用程序主模块 - Windows GUI应用程序
//
// 功能:
// - 窗口创建和管理
// - 消息循环处理
// - UI组件渲染
// - 事件分发
// - 测试UI创建

#include "aether/AetherApplication.h"
#include <iostream>
#include <windowsx.h>

namespace aether {

// Aether应用程序构造函数
AetherApplication::AetherApplication() {
}

// Aether应用程序析构函数
AetherApplication::~AetherApplication() {
    shutdown();
}

// 初始化应用程序
// 参数:
//   hInstance - 应用程序实例句柄
//   nCmdShow - 窗口显示方式
// 返回值: 成功返回true
bool AetherApplication::initialize(HINSTANCE hInstance, int nCmdShow) {
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
    logicLayer_ = std::make_unique<LogicLayer>();
    renderer_ = std::make_unique<Direct2DRenderer>();
    
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
void AetherApplication::shutdown() {
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
void AetherApplication::run() {
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
LRESULT CALLBACK AetherApplication::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    AetherApplication* app = nullptr;
    
    if (uMsg == WM_CREATE) {
        // 从创建参数获取应用程序指针
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<AetherApplication*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        // 从用户数据获取应用程序指针
        app = reinterpret_cast<AetherApplication*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
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
void AetherApplication::createTestUI() {
    if (!logicLayer_) return;
    
    // 创建根容器
    auto root = logicLayer_->createComponent(ComponentType::Container);
    logicLayer_->setProperty(root, PropertyId::Width, PropertyValue(800.0f));
    logicLayer_->setProperty(root, PropertyId::Height, PropertyValue(600.0f));
    
    // 创建按钮1
    auto btn1 = logicLayer_->createComponent(ComponentType::Button, root);
    logicLayer_->setProperty(btn1, PropertyId::X, PropertyValue(100.0f));
    logicLayer_->setProperty(btn1, PropertyId::Y, PropertyValue(100.0f));
    logicLayer_->setProperty(btn1, PropertyId::Width, PropertyValue(200.0f));
    logicLayer_->setProperty(btn1, PropertyId::Height, PropertyValue(50.0f));
    logicLayer_->setProperty(btn1, PropertyId::Text, PropertyValue(std::string("Click Me!")));
    
    // 创建按钮2
    auto btn2 = logicLayer_->createComponent(ComponentType::Button, root);
    logicLayer_->setProperty(btn2, PropertyId::X, PropertyValue(100.0f));
    logicLayer_->setProperty(btn2, PropertyId::Y, PropertyValue(200.0f));
    logicLayer_->setProperty(btn2, PropertyId::Width, PropertyValue(200.0f));
    logicLayer_->setProperty(btn2, PropertyId::Height, PropertyValue(50.0f));
    logicLayer_->setProperty(btn2, PropertyId::Text, PropertyValue(std::string("Submit")));
    
    // 创建文本组件
    auto txt = logicLayer_->createComponent(ComponentType::Text, root);
    logicLayer_->setProperty(txt, PropertyId::X, PropertyValue(100.0f));
    logicLayer_->setProperty(txt, PropertyId::Y, PropertyValue(300.0f));
    logicLayer_->setProperty(txt, PropertyId::Width, PropertyValue(600.0f));
    logicLayer_->setProperty(txt, PropertyId::Height, PropertyValue(100.0f));
    logicLayer_->setProperty(txt, PropertyId::Text, PropertyValue(std::string("Welcome to Aether UI Engine!")));
    
    // 运行一帧更新
    logicLayer_->runFrame();
}

// 绘制回调
void AetherApplication::onPaint() {
    PAINTSTRUCT ps;
    BeginPaint(hwnd_, &ps);
    
    render();
    
    EndPaint(hwnd_, &ps);
}

// 窗口大小改变回调
// 参数: width, height - 新尺寸
void AetherApplication::onResize(int width, int height) {
    if (renderer_) {
        renderer_->resize(width, height);
    }
    InvalidateRect(hwnd_, NULL, FALSE);
}

// 鼠标移动回调
// 参数: x, y - 鼠标坐标
void AetherApplication::onMouseMove(int x, int y) {
    if (logicLayer_) {
        logicLayer_->dispatchMouseMove(static_cast<float>(x), static_cast<float>(y));
    }
}

// 鼠标按下回调
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void AetherApplication::onMouseDown(int x, int y, int button) {
    if (logicLayer_) {
        logicLayer_->dispatchMouseDown(static_cast<float>(x), static_cast<float>(y), button);
    }
}

// 鼠标释放回调
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void AetherApplication::onMouseUp(int x, int y, int button) {
    if (logicLayer_) {
        logicLayer_->dispatchMouseUp(static_cast<float>(x), static_cast<float>(y), button);
    }
}

// 分发点击事件
// 参数:
//   x, y - 鼠标坐标
//   button - 鼠标按钮
void AetherApplication::dispatchClick(int x, int y, int button) {
    if (logicLayer_) {
        logicLayer_->dispatchClick(static_cast<float>(x), static_cast<float>(y), button);
    }
}

// 按键按下回调
// 参数: keyCode - 按键码
void AetherApplication::onKeyDown(int keyCode) {
    if (logicLayer_) {
        logicLayer_->dispatchKeyDown(keyCode);
    }
}

// 按键释放回调
// 参数: keyCode - 按键码
void AetherApplication::onKeyUp(int keyCode) {
    if (logicLayer_) {
        logicLayer_->dispatchKeyUp(keyCode);
    }
}

// 字符输入回调
// 参数: ch - 输入的字符
void AetherApplication::onChar(char ch) {
    if (logicLayer_) {
        logicLayer_->dispatchTextInput(std::string(1, ch));
    }
}

// 渲染UI
void AetherApplication::render() {
    if (!renderer_ || !logicLayer_) return;
    
    renderer_->beginDraw();
    renderer_->clear(Color(0.95f, 0.95f, 0.95f, 1.0f));
    
    auto& storage = logicLayer_->getStorage();
    
    // 遍历所有组件并渲染
    storage.forEach([this, &storage](ComponentHandle handle) {
        auto* entry = storage.getComponent(handle);
        if (!entry) return;
        
        Rect rect(entry->layoutResult.x, entry->layoutResult.y, entry->layoutResult.width, entry->layoutResult.height);
        
        switch (entry->type) {
            case ComponentType::Container:
                // 绘制容器背景
                renderer_->fillRect(rect, Color(0.9f, 0.9f, 0.9f, 1.0f));
                break;
                
            case ComponentType::Button: {
                // 绘制按钮（圆角矩形）
                renderer_->fillRoundedRect(rect, 5.0f, 5.0f, Color(0.2f, 0.5f, 0.9f, 1.0f));
                // 绘制按钮文本
                auto* textProp = entry->properties.getProperty(PropertyId::Text);
                if (textProp) {
                    renderer_->drawText(textProp->get<std::string>(), rect, Color(1.0f, 1.0f, 1.0f, 1.0f), 16.0f);
                }
                break;
            }
                
            case ComponentType::Text: {
                // 绘制文本
                auto* textProp = entry->properties.getProperty(PropertyId::Text);
                if (textProp) {
                    renderer_->drawText(textProp->get<std::string>(), rect, Color(0.0f, 0.0f, 0.0f, 1.0f), 20.0f);
                }
                break;
            }
                
            default:
                break;
        }
    });
    
    renderer_->endDraw();
}

// 更新一帧
void AetherApplication::update() {
    if (logicLayer_) {
        logicLayer_->runFrame();
    }
    
    InvalidateRect(hwnd_, NULL, FALSE);
}

} // namespace aether
