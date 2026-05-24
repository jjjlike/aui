// Direct2DRenderer.cpp
// Direct2D渲染器模块 - Windows平台2D图形渲染
//
// 功能:
// - 颜色、填充矩形
// - 圆角矩形绘制
// - 线条绘制
// - 文本渲染
// - 资源缓存（画刷、文本格式）

#include "aether/Direct2DRenderer.h"
#include "aether/Logger.h"
#include <iostream>
#include <wchar.h>

namespace aether {

// 将Color转换为D2D1_COLOR_F
// 参数: color - 颜色对象
// 返回值: D2D颜色对象
D2D1_COLOR_F Direct2DRenderer::toD2D(const Color& color) {
    return D2D1::ColorF(
        static_cast<float>(color.r) / 255.0f,
        static_cast<float>(color.g) / 255.0f,
        static_cast<float>(color.b) / 255.0f,
        static_cast<float>(color.a) / 255.0f
    );
}

// 将Rect转换为D2D1_RECT_F
// 参数: rect - 矩形对象
// 返回值: D2D矩形对象
D2D1_RECT_F Direct2DRenderer::toD2D(const Rect& rect) {
    return D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

// 将Point转换为D2D1_POINT_2F
// 参数: point - 点对象
// 返回值: D2D点对象
D2D1_POINT_2F Direct2DRenderer::toD2D(const Point& point) {
    return D2D1::Point2F(point.x, point.y);
}

// Direct2D渲染器构造函数
Direct2DRenderer::Direct2DRenderer() {
}

// Direct2D渲染器析构函数
Direct2DRenderer::~Direct2DRenderer() {
    shutdown();
}

// 初始化渲染器
// 参数: hwnd - 窗口句柄
// 返回值: 成功返回true
bool Direct2DRenderer::initialize(HWND hwnd) {
    hwnd_ = hwnd;
    
    if (!createDeviceIndependentResources()) {
        return false;
    }
    
    if (!createDeviceResources()) {
        return false;
    }
    
    return true;
}

// 关闭渲染器
void Direct2DRenderer::shutdown() {
    releaseDeviceResources();
    releaseDeviceIndependentResources();
}

// 创建与设备无关的资源
// 返回值: 成功返回true
bool Direct2DRenderer::createDeviceIndependentResources() {
    if (factory_ || writeFactory_ || wicFactory_) {
        releaseDeviceIndependentResources();
    }
    
    // 创建D2D工厂
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory_);
    if (FAILED(hr)) {
        std::cerr << "Failed to create D2D1 factory" << std::endl;
        return false;
    }
    
    // 创建DirectWrite工厂
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&writeFactory_));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DWrite factory" << std::endl;
        return false;
    }
    
    // 创建WIC工厂
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory_));
    if (FAILED(hr)) {
        std::cerr << "Failed to create WIC factory" << std::endl;
        return false;
    }
    
    return true;
}

// 创建设备资源
// 返回值: 成功返回true
bool Direct2DRenderer::createDeviceResources() {
    RECT clientRect;
    GetClientRect(hwnd_, &clientRect);
    
    D2D1_SIZE_U size = D2D1::SizeU(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
    
    // 创建渲染目标
    HRESULT hr = factory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd_, size),
        &renderTarget_
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create render target" << std::endl;
        return false;
    }
    
    return true;
}

// 释放设备资源
void Direct2DRenderer::releaseDeviceResources() {
    // 释放画刷缓存
    for (auto& pair : brushCache_) {
        if (pair.second) pair.second->Release();
    }
    brushCache_.clear();
    
    // 释放文本格式缓存
    for (auto& pair : textFormatCache_) {
        if (pair.second) pair.second->Release();
    }
    textFormatCache_.clear();
    
    // 释放渲染目标
    if (renderTarget_) {
        renderTarget_->Release();
        renderTarget_ = nullptr;
    }
}

// 释放与设备无关的资源
void Direct2DRenderer::releaseDeviceIndependentResources() {
    if (wicFactory_) {
        wicFactory_->Release();
        wicFactory_ = nullptr;
    }
    
    if (writeFactory_) {
        writeFactory_->Release();
        writeFactory_ = nullptr;
    }
    
    if (factory_) {
        factory_->Release();
        factory_ = nullptr;
    }
}

// 开始绘制
void Direct2DRenderer::beginDraw() {
    Logger::getInstance().debug("[Direct2DRenderer] beginDraw 调用");
    if (renderTarget_) {
        renderTarget_->BeginDraw();
    } else {
        Logger::getInstance().warning("[Direct2DRenderer] beginDraw 失败：renderTarget_ 为空");
    }
}

// 结束绘制
void Direct2DRenderer::endDraw() {
    Logger::getInstance().debug("[Direct2DRenderer] endDraw 调用");
    if (renderTarget_) {
        renderTarget_->EndDraw();
    } else {
        Logger::getInstance().warning("[Direct2DRenderer] endDraw 失败：renderTarget_ 为空");
    }
}

// 调整大小
// 参数: width, height - 新尺寸
void Direct2DRenderer::resize(int width, int height) {
    if (renderTarget_) {
        releaseDeviceResources();
        createDeviceResources();
    }
}

// 清空画布
// 参数: color - 背景颜色
void Direct2DRenderer::clear(const Color& color) {
    if (renderTarget_) {
        renderTarget_->Clear(toD2D(color));
    }
}

// 获取或创建画刷
// 参数: color - 颜色
// 返回值: 画刷指针
ID2D1SolidColorBrush* Direct2DRenderer::getBrush(const Color& color) {
    uint32_t key = color.toARGB();
    auto it = brushCache_.find(key);
    if (it != brushCache_.end()) {
        return it->second;
    }
    
    // 创建新画刷
    ID2D1SolidColorBrush* brush = nullptr;
    HRESULT hr = renderTarget_->CreateSolidColorBrush(toD2D(color), &brush);
    if (SUCCEEDED(hr)) {
        brushCache_[key] = brush;
        return brush;
    }
    return nullptr;
}

// 绘制矩形边框
// 参数:
//   rect - 矩形
//   color - 颜色
//   strokeWidth - 线宽
void Direct2DRenderer::drawRect(const Rect& rect, const Color& color, float strokeWidth) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        renderTarget_->DrawRectangle(toD2D(rect), brush, strokeWidth);
    }
}

// 填充矩形
// 参数:
//   rect - 矩形
//   color - 颜色
void Direct2DRenderer::fillRect(const Rect& rect, const Color& color) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        renderTarget_->FillRectangle(toD2D(rect), brush);
    }
}

// 绘制圆角矩形边框
// 参数:
//   rect - 矩形
//   radiusX, radiusY - 圆角半径
//   color - 颜色
//   strokeWidth - 线宽
void Direct2DRenderer::drawRoundedRect(const Rect& rect, float radiusX, float radiusY, const Color& color, float strokeWidth) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(toD2D(rect), radiusX, radiusY);
        renderTarget_->DrawRoundedRectangle(roundedRect, brush, strokeWidth);
    }
}

// 填充圆角矩形
// 参数:
//   rect - 矩形
//   radiusX, radiusY - 圆角半径
//   color - 颜色
void Direct2DRenderer::fillRoundedRect(const Rect& rect, float radiusX, float radiusY, const Color& color) {
    Logger::getInstance().debug("[Direct2DRenderer] fillRoundedRect 调用: "
        "rect=(" + std::to_string(static_cast<int>(rect.x)) + "," + std::to_string(static_cast<int>(rect.y)) + "," 
        + std::to_string(static_cast<int>(rect.width)) + "x" + std::to_string(static_cast<int>(rect.height)) + ") "
        "radiusX=" + std::to_string(radiusX) + " radiusY=" + std::to_string(radiusY));
    
    if (!renderTarget_) {
        Logger::getInstance().warning("[Direct2DRenderer] fillRoundedRect 失败：renderTarget_ 为空");
        return;
    }
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(toD2D(rect), radiusX, radiusY);
        renderTarget_->FillRoundedRectangle(roundedRect, brush);
        Logger::getInstance().debug("[Direct2DRenderer] fillRoundedRect 绘制完成");
    } else {
        Logger::getInstance().warning("[Direct2DRenderer] fillRoundedRect 失败：获取画刷失败");
    }
}

// 绘制线条
// 参数:
//   p1, p2 - 起点和终点
//   color - 颜色
//   strokeWidth - 线宽
void Direct2DRenderer::drawLine(const Point& p1, const Point& p2, const Color& color, float strokeWidth) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        renderTarget_->DrawLine(toD2D(p1), toD2D(p2), brush, strokeWidth);
    }
}

// 绘制文本
// 参数:
//   text - 文本内容
//   rect - 文本区域
//   color - 文本颜色
//   fontSize - 字体大小
//   fontName - 字体名称
void Direct2DRenderer::drawText(const std::string& text, const Rect& rect, const Color& color, float fontSize, const std::string& fontName) {
    if (!renderTarget_ || !writeFactory_) return;
    
    // 查找或创建文本格式
    std::string key = fontName + "_" + std::to_string(fontSize);
    IDWriteTextFormat* textFormat = nullptr;
    
    auto it = textFormatCache_.find(key);
    if (it != textFormatCache_.end()) {
        textFormat = it->second;
    } else {
        // 转换字体名称为宽字符
        int len = MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, NULL, 0);
        std::wstring wFontName(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, &wFontName[0], len);
        
        // 创建文本格式
        HRESULT hr = writeFactory_->CreateTextFormat(
            wFontName.c_str(),
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"en-us",
            &textFormat
        );
        
        if (SUCCEEDED(hr)) {
            // 设置文本对齐方式
            textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            textFormatCache_[key] = textFormat;
        }
    }
    
    if (!textFormat) return;
    
    // 转换文本为宽字符
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    std::wstring wText(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wText[0], len);
    
    // 绘制文本
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        renderTarget_->DrawText(
            wText.c_str(),
            static_cast<UINT32>(wText.length()),
            textFormat,
            toD2D(rect),
            brush
        );
    }
}

} // namespace aether
