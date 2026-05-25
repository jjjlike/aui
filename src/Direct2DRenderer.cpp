// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


// JDirect2DRenderer.cpp
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

namespace jaether {

// 将Color转换为D2D1_COLOR_F
// 参数: color - 颜色对象
// 返回值: D2D颜色对象
D2D1_COLOR_F JDirect2DRenderer::toD2D(const JColor& color) {
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
D2D1_RECT_F JDirect2DRenderer::toD2D(const JRect& rect) {
    return D2D1::RectF(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

// 将Point转换为D2D1_POINT_2F
// 参数: point - 点对象
// 返回值: D2D点对象
D2D1_POINT_2F JDirect2DRenderer::toD2D(const JPoint& point) {
    return D2D1::Point2F(point.x, point.y);
}

// Direct2D渲染器构造函数
JDirect2DRenderer::JDirect2DRenderer() {
}

// Direct2D渲染器析构函数
// 注意：不在析构函数中调用shutdown()，避免与AetherApplication::shutdown()
// 中已经调用的renderer_->shutdown()产生双重释放
// shutdown()由外部显式调用（或通过JAetherApplication::shutdown()间接调用）
JDirect2DRenderer::~JDirect2DRenderer() {
    // 仅当外部未调用shutdown()时做清理（如测试中直接delete renderer_）
    if (!shutdown_) {
        shutdown();
    }
}

// 初始化渲染器
// 参数: hwnd - 窗口句柄
// 返回值: 成功返回true
bool JDirect2DRenderer::initialize(HWND hwnd) {
    hwnd_ = hwnd;
    
    if (!createDeviceIndependentResources()) {
        return false;
    }
    
    if (!createDeviceResources()) {
        return false;
    }
    
    return true;
}

// 关闭渲染器（幂等操作，可安全重复调用）
void JDirect2DRenderer::shutdown() {
    // 防止双重释放：如果已经关闭过，直接返回
    if (shutdown_) return;
    shutdown_ = true;
    
    releaseDeviceResources();
    releaseDeviceIndependentResources();
}

// 创建与设备无关的资源
// 返回值: 成功返回true
bool JDirect2DRenderer::createDeviceIndependentResources() {
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
bool JDirect2DRenderer::createDeviceResources() {
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
    
    renderTarget_->GetDpi(&dpiX_, &dpiY_);
    
    return true;
}

// 释放设备资源
void JDirect2DRenderer::releaseDeviceResources() {
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
    
    // 释放带对齐方式的文本格式缓存
    for (auto& pair : alignedTextFormatCache_) {
        if (pair.second) pair.second->Release();
    }
    alignedTextFormatCache_.clear();
    
    // 释放渲染目标
    if (renderTarget_) {
        renderTarget_->Release();
        renderTarget_ = nullptr;
    }
}

// 释放与设备无关的资源
void JDirect2DRenderer::releaseDeviceIndependentResources() {
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
void JDirect2DRenderer::beginDraw() {
    JLogger::getInstance().debug("[JDirect2DRenderer] beginDraw 调用");
    if (renderTarget_) {
        renderTarget_->BeginDraw();
    } else {
        JLogger::getInstance().warning("[JDirect2DRenderer] beginDraw 失败：renderTarget_ 为空");
    }
}

// 结束绘制
void JDirect2DRenderer::endDraw() {
    JLogger::getInstance().debug("[JDirect2DRenderer] endDraw 调用");
    if (renderTarget_) {
        renderTarget_->EndDraw();
    } else {
        JLogger::getInstance().warning("[JDirect2DRenderer] endDraw 失败：renderTarget_ 为空");
    }
}

// 调整大小
// 参数: width, height - 新尺寸
void JDirect2DRenderer::resize(int width, int height) {
    if (renderTarget_) {
        releaseDeviceResources();
        createDeviceResources();
    }
}

// 清空画布
// 参数: color - 背景颜色
void JDirect2DRenderer::clear(const JColor& color) {
    if (renderTarget_) {
        renderTarget_->Clear(toD2D(color));
    }
}

// 获取或创建画刷
// 参数: color - 颜色
// 返回值: 画刷指针
ID2D1SolidColorBrush* JDirect2DRenderer::getBrush(const JColor& color) {
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
void JDirect2DRenderer::drawRect(const JRect& rect, const JColor& color, float strokeWidth) {
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
void JDirect2DRenderer::fillRect(const JRect& rect, const JColor& color) {
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
void JDirect2DRenderer::drawRoundedRect(const JRect& rect, float radiusX, float radiusY, const JColor& color, float strokeWidth) {
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
void JDirect2DRenderer::fillRoundedRect(const JRect& rect, float radiusX, float radiusY, const JColor& color) {
    JLogger::getInstance().debug("[JDirect2DRenderer] fillRoundedRect 调用: "
        "rect=(" + std::to_string(static_cast<int>(rect.x)) + "," + std::to_string(static_cast<int>(rect.y)) + "," 
        + std::to_string(static_cast<int>(rect.width)) + "x" + std::to_string(static_cast<int>(rect.height)) + ") "
        "radiusX=" + std::to_string(radiusX) + " radiusY=" + std::to_string(radiusY));
    
    if (!renderTarget_) {
        JLogger::getInstance().warning("[JDirect2DRenderer] fillRoundedRect 失败：renderTarget_ 为空");
        return;
    }
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(toD2D(rect), radiusX, radiusY);
        renderTarget_->FillRoundedRectangle(roundedRect, brush);
        JLogger::getInstance().debug("[JDirect2DRenderer] fillRoundedRect 绘制完成");
    } else {
        JLogger::getInstance().warning("[JDirect2DRenderer] fillRoundedRect 失败：获取画刷失败");
    }
}

// 绘制线条
// 参数:
//   p1, p2 - 起点和终点
//   color - 颜色
//   strokeWidth - 线宽
void JDirect2DRenderer::drawLine(const JPoint& p1, const JPoint& p2, const JColor& color, float strokeWidth) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        renderTarget_->DrawLine(toD2D(p1), toD2D(p2), brush, strokeWidth);
    }
}

// 绘制文本（委托给 drawTextAligned，保持向后兼容）
// 参数: text, rect, color, fontSize, fontName
void JDirect2DRenderer::drawText(const std::string& text, const JRect& rect, const JColor& color, float fontSize, const std::string& fontName) {
    drawTextAligned(text, rect, color, fontSize, fontName, JTextAlignment::Center);
}

// 绘制文本（支持水平对齐方式）
// 参数:
//   text - 文本内容
//   rect - 文本区域
//   color - 文本颜色
//   fontSize - 字体大小
//   fontName - 字体名称
//   alignment - 水平对齐方式（Left/Center/Right）
void JDirect2DRenderer::drawTextAligned(const std::string& text, const JRect& rect, const JColor& color,
                                         float fontSize, const std::string& fontName, JTextAlignment alignment) {
    if (!renderTarget_ || !writeFactory_) return;
    
    // 获取或创建带对齐方式的文本格式
    IDWriteTextFormat* textFormat = getTextFormat(fontSize, fontName, alignment);
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

// 测量文本的像素尺寸（DIP单位）
// 使用 IDWriteTextLayout 进行精确测量
JSize JDirect2DRenderer::measureText(const std::string& text, float fontSize, const std::string& fontName) const {
    if (!writeFactory_) return JSize{0, 0};
    if (text.empty()) return JSize{0, 0};
    
    // 转换文本为宽字符
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    std::wstring wText(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wText[0], len);
    
    // 获取文本格式（不需要缓存，这里用局部创建）
    // 转换字体名称为宽字符
    int fnLen = MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, NULL, 0);
    std::wstring wFontName(fnLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, &wFontName[0], fnLen);
    
    IDWriteTextFormat* format = nullptr;
    HRESULT hr = writeFactory_->CreateTextFormat(
        wFontName.c_str(), NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        fontSize, L"en-us", &format);
    
    if (FAILED(hr) || !format) return JSize{0, 0};
    
    // 创建 TextLayout 进行测量（使用足够大的宽度和高度）
    IDWriteTextLayout* layout = nullptr;
    hr = writeFactory_->CreateTextLayout(
        wText.c_str(), static_cast<UINT32>(wText.length()),
        format, 1e6f, 1e6f, &layout);
    
    format->Release();
    
    if (FAILED(hr) || !layout) return JSize{0, 0};
    
    DWRITE_TEXT_METRICS metrics;
    layout->GetMetrics(&metrics);
    layout->Release();
    
    return JSize{metrics.width, metrics.height};
}

// 绘制椭圆边框
void JDirect2DRenderer::drawEllipse(const JRect& rect, const JColor& color, float strokeWidth) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        // 椭圆中心 = 矩形中心，半径 = 矩形半宽/半高
        D2D1_ELLIPSE ellipse;
        ellipse.point = D2D1::Point2F(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f);
        ellipse.radiusX = rect.width / 2.0f;
        ellipse.radiusY = rect.height / 2.0f;
        renderTarget_->DrawEllipse(ellipse, brush, strokeWidth);
    }
}

// 填充椭圆
void JDirect2DRenderer::fillEllipse(const JRect& rect, const JColor& color) {
    if (!renderTarget_) return;
    
    ID2D1SolidColorBrush* brush = getBrush(color);
    if (brush) {
        D2D1_ELLIPSE ellipse;
        ellipse.point = D2D1::Point2F(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f);
        ellipse.radiusX = rect.width / 2.0f;
        ellipse.radiusY = rect.height / 2.0f;
        renderTarget_->FillEllipse(ellipse, brush);
    }
}

// 获取或创建带对齐方式的文本格式
IDWriteTextFormat* JDirect2DRenderer::getTextFormat(float fontSize, const std::string& fontName,
                                                     JTextAlignment alignment) {
    // 构建缓存键：字体名_字号_对齐方式
    std::string cacheKey = fontName + "_" + std::to_string(fontSize) + "_" + std::to_string(static_cast<int>(alignment));
    
    // 先从带对齐的缓存中查找
    auto it = alignedTextFormatCache_.find(cacheKey);
    if (it != alignedTextFormatCache_.end()) {
        return it->second;
    }
    
    // 转换字体名称为宽字符
    int len = MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, NULL, 0);
    std::wstring wFontName(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, &wFontName[0], len);
    
    // 创建文本格式
    IDWriteTextFormat* textFormat = nullptr;
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
        // 根据对齐方式设置文本对齐
        switch (alignment) {
            case JTextAlignment::Left:
                textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                break;
            case JTextAlignment::Center:
                textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                break;
            case JTextAlignment::Right:
                textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                break;
        }
        textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        alignedTextFormatCache_[cacheKey] = textFormat;
    }
    
    return textFormat;
}

} // namespace jaether
