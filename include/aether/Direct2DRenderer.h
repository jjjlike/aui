#pragma once

#include "aether/types.h"
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <memory>
#include <string>
#include <unordered_map>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace aether {

/**
 * Direct2D渲染器类
 * 
 * 使用Direct2D进行UI渲染
 * 提供基本绘图功能
 */
class Direct2DRenderer {
public:
    /**
     * 构造函数
     */
    Direct2DRenderer();
    
    /**
     * 析构函数
     */
    ~Direct2DRenderer();
    
    /**
     * 初始化渲染器
     * @param hwnd 窗口句柄
     * @return 是否初始化成功
     */
    bool initialize(HWND hwnd);
    
    /**
     * 关闭渲染器
     * 释放所有资源
     */
    void shutdown();
    
    /**
     * 开始绘制
     * 必须在绘制前调用
     */
    void beginDraw();
    
    /**
     * 结束绘制
     * 必须在绘制后调用
     */
    void endDraw();
    
    /**
     * 调整渲染器大小
     * @param width 宽度
     * @param height 高度
     */
    void resize(int width, int height);
    
    /**
     * 清除屏幕
     * @param color 清除颜色
     */
    void clear(const Color& color);
    
    /**
     * 绘制矩形（边框）
     * @param rect 矩形区域
     * @param color 边框颜色
     * @param strokeWidth 线条宽度
     */
    void drawRect(const Rect& rect, const Color& color, float strokeWidth = 1.0f);
    
    /**
     * 填充矩形
     * @param rect 矩形区域
     * @param color 填充颜色
     */
    void fillRect(const Rect& rect, const Color& color);
    
    /**
     * 绘制圆角矩形（边框）
     * @param rect 矩形区域
     * @param radiusX X方向圆角半径
     * @param radiusY Y方向圆角半径
     * @param color 边框颜色
     * @param strokeWidth 线条宽度
     */
    void drawRoundedRect(const Rect& rect, float radiusX, float radiusY, const Color& color, float strokeWidth = 1.0f);
    
    /**
     * 填充圆角矩形
     * @param rect 矩形区域
     * @param radiusX X方向圆角半径
     * @param radiusY Y方向圆角半径
     * @param color 填充颜色
     */
    void fillRoundedRect(const Rect& rect, float radiusX, float radiusY, const Color& color);
    
    /**
     * 绘制直线
     * @param p1 起点
     * @param p2 终点
     * @param color 线条颜色
     * @param strokeWidth 线条宽度
     */
    void drawLine(const Point& p1, const Point& p2, const Color& color, float strokeWidth = 1.0f);
    
    /**
     * 绘制文本
     * @param text 文本内容
     * @param rect 文本区域
     * @param color 文本颜色
     * @param fontSize 字体大小
     * @param fontName 字体名称
     */
    void drawText(const std::string& text, const Rect& rect, const Color& color, float fontSize = 16.0f, const std::string& fontName = "Segoe UI");
    
    /**
     * 获取渲染目标
     * @return Direct2D渲染目标指针
     */
    ID2D1HwndRenderTarget* getRenderTarget() { return renderTarget_; }
    
    /**
     * 获取当前DPI X方向
     * @return DPI值
     */
    float getDpiX() const { return dpiX_; }
    
    /**
     * 获取当前DPI Y方向
     * @return DPI值
     */
    float getDpiY() const { return dpiY_; }
    
private:
    /**
     * 将Color转换为Direct2D颜色
     * @param color Color对象
     * @return Direct2D颜色
     */
    static D2D1_COLOR_F toD2D(const Color& color);
    
    /**
     * 将Rect转换为Direct2D矩形
     * @param rect Rect对象
     * @return Direct2D矩形
     */
    static D2D1_RECT_F toD2D(const Rect& rect);
    
    /**
     * 将Point转换为Direct2D点
     * @param point Point对象
     * @return Direct2D点
     */
    static D2D1_POINT_2F toD2D(const Point& point);
    
    /**
     * 获取或创建画刷
     * @param color 颜色
     * @return 画刷指针
     */
    ID2D1SolidColorBrush* getBrush(const Color& color);
    
    /**
     * 创建设备无关资源
     * @return 是否成功
     */
    bool createDeviceIndependentResources();
    
    /**
     * 创建设备相关资源
     * @return 是否成功
     */
    bool createDeviceResources();
    
    /**
     * 释放设备相关资源
     */
    void releaseDeviceResources();
    
    /**
     * 释放设备无关资源
     */
    void releaseDeviceIndependentResources();
    
    ID2D1Factory* factory_ = nullptr;                    // Direct2D工厂
    ID2D1HwndRenderTarget* renderTarget_ = nullptr;      // 渲染目标
    IDWriteFactory* writeFactory_ = nullptr;             // DirectWrite工厂
    IWICImagingFactory* wicFactory_ = nullptr;           // WIC工厂
    
    std::unordered_map<uint32_t, ID2D1SolidColorBrush*> brushCache_;  // 画刷缓存
    std::unordered_map<std::string, IDWriteTextFormat*> textFormatCache_;  // 文本格式缓存
    
    float dpiX_ = 96.0f;  // 当前DPI X方向
    float dpiY_ = 96.0f;  // 当前DPI Y方向
    
    HWND hwnd_ = nullptr;  // 窗口句柄
};

}
