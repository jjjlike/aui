#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

namespace jaether {

/**
 * 日志级别枚举
 */
enum class JLogLevel {
    Debug,   // 调试信息
    Info,    // 一般信息
    Warning, // 警告
    Error    // 错误
};

/**
 * 运行时日志类
 * 
 * 提供统一的日志输出接口
 * 用于追踪程序运行时的关键事件
 */
class JLogger {
public:
    /**
     * 获取Logger单例实例
     * @return Logger引用
     */
    static JLogger& getInstance() {
        static JLogger instance;
        return instance;
    }

    /**
     * 设置日志级别
     * @param level 日志级别
     */
    void setLevel(JLogLevel level) {
        level_ = level;
    }

    /**
     * 启用/禁用日志输出到文件
     * @param enabled 是否启用
     * @param filename 文件名
     */
    void enableFileOutput(bool enabled, const std::string& filename = "aether.log") {
        fileOutputEnabled_ = enabled;
        logFilename_ = filename;
    }

    /**
     * 输出调试日志
     * @param message 日志消息
     * @param file 来源文件
     * @param line 行号
     */
    void debug(const std::string& message, const char* file = "", int line = 0) {
        log(JLogLevel::Debug, message, file, line);
    }

    /**
     * 输出信息日志
     * @param message 日志消息
     * @param file 来源文件
     * @param line 行号
     */
    void info(const std::string& message, const char* file = "", int line = 0) {
        log(JLogLevel::Info, message, file, line);
    }

    /**
     * 输出警告日志
     * @param message 日志消息
     * @param file 来源文件
     * @param line 行号
     */
    void warning(const std::string& message, const char* file = "", int line = 0) {
        log(JLogLevel::Warning, message, file, line);
    }

    /**
     * 输出错误日志
     * @param message 日志消息
     * @param file 来源文件
     * @param line 行号
     */
    void error(const std::string& message, const char* file = "", int line = 0) {
        log(JLogLevel::Error, message, file, line);
    }

    /**
     * 输出组件创建日志
     * @param handle 组件句柄
     * @param type 组件类型
     * @param parent 父组件句柄
     */
    void logComponentCreate(JComponentHandle handle, JComponentType type, JComponentHandle parent) {
        std::string msg = "组件创建: handle=" + std::to_string(handle.index) + 
                         "/" + std::to_string(handle.generation) +
                         " type=" + std::to_string(static_cast<int>(type)) +
                         " parent=" + std::to_string(parent.index);
        info(msg);
    }

    /**
     * 输出组件销毁日志
     * @param handle 组件句柄
     */
    void logComponentDestroy(JComponentHandle handle) {
        std::string msg = "组件销毁: handle=" + std::to_string(handle.index) + 
                         "/" + std::to_string(handle.generation);
        info(msg);
    }

    /**
     * 输出布局计算日志
     * @param handle 组件句柄
     * @param x X坐标
     * @param y Y坐标
     * @param width 宽度
     * @param height 高度
     */
    void logLayout(JComponentHandle handle, float x, float y, float width, float height) {
        std::string msg = "布局计算: handle=" + std::to_string(handle.index) +
                         " rect=(" + std::to_string(static_cast<int>(x)) + "," + 
                         std::to_string(static_cast<int>(y)) + "," +
                         std::to_string(static_cast<int>(width)) + "x" + 
                         std::to_string(static_cast<int>(height)) + ")";
        debug(msg);
    }

    /**
     * 输出事件分发日志
     * @param eventType 事件类型
     * @param x X坐标
     * @param y Y坐标
     */
    void logEvent(const std::string& eventType, float x, float y) {
        std::string msg = "事件: " + eventType + 
                         " pos=(" + std::to_string(static_cast<int>(x)) + "," +
                         std::to_string(static_cast<int>(y)) + ")";
        debug(msg);
    }

    /**
     * 输出渲染日志
     * @param handle 组件句柄
     * @param type 组件类型
     */
    void logRender(JComponentHandle handle, JComponentType type) {
        std::string msg = "渲染: handle=" + std::to_string(handle.index) +
                         " type=" + std::to_string(static_cast<int>(type));
        debug(msg);
    }

    /**
     * 输出属性设置日志
     * @param handle 组件句柄
     * @param propertyId 属性ID
     * @param value 属性值
     */
    void logPropertySet(JComponentHandle handle, int propertyId, const std::string& value) {
        std::string msg = "属性设置: handle=" + std::to_string(handle.index) +
                         " property=" + std::to_string(propertyId) +
                         " value=" + value;
        debug(msg);
    }

private:
    /**
     * 私有构造函数
     */
    JLogger() : level_(JLogLevel::Debug), fileOutputEnabled_(false) {}

    /**
     * 日志输出函数
     */
    void log(JLogLevel level, const std::string& message, const char* file, int line) {
        if (level < level_) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm;
        localtime_s(&tm, &time);

        std::cout << "[" << std::put_time(&tm, "%H:%M:%S") 
                  << "." << std::setfill('0') << std::setw(3) << ms.count() << "]";

        switch (level) {
            case JLogLevel::Debug:
                std::cout << "[DEBUG]";
                break;
            case JLogLevel::Info:
                std::cout << "[INFO] ";
                break;
            case JLogLevel::Warning:
                std::cout << "[WARN] ";
                break;
            case JLogLevel::Error:
                std::cout << "[ERROR]";
                break;
        }

        std::cout << " " << message;
        
        if (file && file[0] != '\0') {
            std::cout << " (" << file << ":" << line << ")";
        }
        
        std::cout << std::endl;

        if (fileOutputEnabled_) {
            writeToFileInternal(message);
        }
    }

    /**
     * 写入文件
     */
    void writeToFileInternal(const std::string& message) {
        static FILE* file = nullptr;
        if (!file) {
            file = fopen(logFilename_.c_str(), "w");
        }
        if (file) {
            fprintf(file, "%s\n", message.c_str());
            fflush(file);
        }
    }

    JLogLevel level_;
    bool fileOutputEnabled_;
    std::string logFilename_;
};

#define AETHER_LOG_DEBUG(msg) jaether::JLogger::getInstance().debug(msg, __FILE__, __LINE__)
#define AETHER_LOG_INFO(msg) jaether::JLogger::getInstance().info(msg, __FILE__, __LINE__)
#define AETHER_LOG_WARNING(msg) jaether::JLogger::getInstance().warning(msg, __FILE__, __LINE__)
#define AETHER_LOG_ERROR(msg) jaether::JLogger::getInstance().error(msg, __FILE__, __LINE__)

} // namespace jaether
