#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <array>
#include <functional>

namespace aether {

// 类型别名定义

/**
 * 组件ID类型
 * 唯一标识一个组件
 */
using ComponentId = uint32_t;

/**
 * 世代号类型
 * 用于检测组件句柄有效性
 */
using Generation = uint32_t;

/**
 * 无效组件ID常量
 */
constexpr ComponentId INVALID_COMPONENT_ID = 0;

/**
 * 无效世代号常量
 */
constexpr Generation INVALID_GENERATION = 0;

/**
 * 二维点结构体
 * 
 * 表示平面上的一个点
 */
struct Point {
    float x = 0;
    float y = 0;
    
    /**
     * 检查点是否包含另一个点
     * @param px x坐标
     * @param py y坐标
     * @return true如果包含
     */
    bool contains(float px, float py) const {
        return px >= x && px < x + 1 && py >= y && py < y + 1;
    }
};

/**
 * 尺寸结构体
 * 
 * 表示宽度和高度
 */
struct Size {
    float width = 0;
    float height = 0;
};

/**
 * 矩形结构体
 * 
 * 表示一个矩形区域
 * 由左上角坐标 + 宽度高度
 */
struct Rect {
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
    
    Rect() = default;
    Rect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    /**
     * 获取左边界坐标
     * @return 左边界x坐标
     */
    float left() const { return x; }
    
    /**
     * 获取上边界坐标
     * @return 上边界y坐标
     */
    float top() const { return y; }
    
    /**
     * 获取右边界坐标
     * @return 右边界x坐标
     */
    float right() const { return x + width; }
    
    /**
     * 获取下边界坐标
     * @return 下边界y坐标
     */
    float bottom() const { return y + height; }
    
    /**
     * 检查点是否在矩形内
     * @param px x坐标
     * @param py y坐标
     * @return true如果点在矩形内
     */
    bool contains(float px, float py) const {
        return px >= x && px < right() && py >= y && py < bottom();
    }
    
    /**
     * 检查点是否在矩形内
     * @param p 点
     * @return true如果点在矩形内
     */
    bool contains(Point p) const { return contains(p.x, p.y); }
    
    /**
     * 创建一个无限大的矩形
     * @return 无限大的矩形
     */
    static Rect infinite() {
        return {-1e6f, -1e6f, 2e6f, 2e6f};
    }
    
    /**
     * 从两个点创建矩形
     * @param x1 左边界
     * @param y1 上边界
     * @param x2 右边界
     * @param y2 下边界
     * @return 矩形
     */
    static Rect fromPoints(float x1, float y1, float x2, float y2) {
        return {x1, y1, x2 - x1, y2 - y1};
    }
};

/**
 * 颜色结构体
 * 
 * RGBA颜色
 * r, g, b, a 取值范围 0-255
 */
struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    
    Color() = default;
    
    /**
     * 从RGBA分量创建颜色
     * @param r 红色分量
     * @param g 绿色分量
     * @param b 蓝色分量
     * @param a alpha分量（默认255）
     */
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    
    /**
     * 从0-1浮点数创建颜色
     * @param r 红色分量
     * @param g 绿色分量
     * @param b 蓝色分量
     * @param a alpha分量（默认1.0）
     */
    Color(float r, float g, float b, float a = 1.0f) 
        : r(static_cast<uint8_t>(r * 255)), 
          g(static_cast<uint8_t>(g * 255)), 
          b(static_cast<uint8_t>(b * 255)), 
          a(static_cast<uint8_t>(a * 255)) {}
    
    /**
     * 从ARGB整数创建颜色
     * @param argb ARGB格式32位整数
     * @return 颜色
     */
    static Color fromARGB(uint32_t argb) {
        return {
            static_cast<uint8_t>((argb >> 16) & 0xFF),
            static_cast<uint8_t>((argb >> 8) & 0xFF),
            static_cast<uint8_t>(argb & 0xFF),
            static_cast<uint8_t>((argb >> 24) & 0xFF)
        };
    }
    
    /**
     * 转换为ARGB整数
     * @return ARGB格式32位整数
     */
    uint32_t toARGB() const {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8) |
               static_cast<uint32_t>(b);
    }
};

/**
 * 组件类型枚举
 */
enum class ComponentType : uint8_t {
    Container,    // 容器
    Button,        // 按钮
    Text,          // 文本
    Input,         // 输入框
    ScrollView,    // 滚动视图
    Image,         // 图片
    Custom         // 自定义
};

/**
 * 组件句柄结构体
 * 
 * 使用索引 + 世代号
 * 防止访问时可以验证有效性
 */
struct ComponentHandle {
    int32_t index = -1;
    Generation generation = 0;
    
    /**
     * 检查句柄是否有效
     * @return true如果句柄有效
     */
    bool isValid() const { return index >= 0 && generation != INVALID_GENERATION; }
    
    /**
     * 隐式转换为bool
     * @return true如果句柄有效
     */
    explicit operator bool() const { return isValid(); }
    
    /**
     * 相等比较
     * @param other 另一个句柄
     * @return true如果相等
     */
    bool operator==(const ComponentHandle& other) const {
        return index == other.index && generation == other.generation;
    }
    
    /**
     * 不相等比较
     * @param other 另一个句柄
     * @return true如果不相等
     */
    bool operator!=(const ComponentHandle& other) const {
        return !(*this == other);
    }
};

/**
 * Flex布局主轴方向
 */
enum class FlexDirection : uint8_t {
    Row,       // 水平方向
    Column     // 垂直方向
};

/**
 * Flex换行方式
 */
enum class FlexWrap : uint8_t {
    NoWrap,  // 不换行
    Wrap       // 换行
};

/**
 * Flex主轴对齐方式
 */
enum class JustifyContent : uint8_t {
    FlexStart,     // 起始对齐
    Center,         // 居中对齐
    FlexEnd,        // 结尾对齐
    SpaceBetween,  // 两端对齐，元素间间隔均匀
    SpaceAround     // 元素周围间隔均匀
};

/**
 * Flex交叉轴对齐方式（单行）
 */
enum class AlignItems : uint8_t {
    FlexStart,     // 起始对齐
    Center,         // 居中对齐
    FlexEnd,        // 结尾对齐
    Stretch         // 拉伸
};

/**
 * Flex交叉轴对齐方式（多行）
 */
enum class AlignContent : uint8_t {
    FlexStart,     // 起始对齐
    Center,         // 居中对齐
    FlexEnd,        // 结尾对齐
    SpaceBetween,  // 两端对齐
    SpaceAround,    // 元素周围间隔均匀
    Stretch         // 拉伸
};

/**
 * 布局引擎模式
 */
enum class LayoutEngineMode {
    Normal,         // 正常模式
    Test            // 测试模式
};

}
