#pragma once

#include <string>
#include <vector>
#include <functional>
#include <windows.h>

namespace aether {

/**
 * 消息回调类型
 */
using MessageCallback = std::function<void(const std::string&)>;

/**
 * IPC客户端类
 * 
 * 用于与IPC服务器通信
 * 支持命名管道通信
 */
class IPCClient {
public:
    /**
     * 构造函数
     */
    IPCClient();
    
    /**
     * 析构函数
     */
    ~IPCClient();
    
    /**
     * 连接到服务器
     * @param pipeName 管道名称
     * @return 是否连接成功
     */
    bool connect(const std::string& pipeName = "\\\\.\\pipe\\aether_ui");
    
    /**
     * 断开连接
     */
    void disconnect();
    
    /**
     * 检查是否已连接
     * @return 如果已连接返回true
     */
    bool isConnected() const;
    
    /**
     * 发送消息
     * @param message 消息字符串
     * @return 是否发送成功
     */
    bool sendMessage(const std::string& message);
    
    /**
     * 接收消息（阻塞）
     * @return 接收到的消息
     */
    std::string receiveMessage();
    
    /**
     * 设置消息回调
     * @param callback 回调函数
     */
    void setMessageCallback(MessageCallback callback) { messageCallback_ = std::move(callback); }
    
    /**
     * 开始异步读取
     * 接收到消息时会调用回调函数
     */
    void startAsyncRead();
    
    /**
     * 停止异步读取
     */
    void stopAsyncRead();
    
private:
    /**
     * 异步读取线程函数
     * @param param 线程参数
     * @return 线程退出码
     */
    static DWORD WINAPI asyncReadThread(LPVOID param);
    
    /**
     * 读取循环
     */
    void readLoop();
    
    HANDLE pipeHandle_ = INVALID_HANDLE_VALUE;  // 管道句柄
    bool connected_ = false;                    // 连接状态
    bool reading_ = false;                       // 是否正在读取
    HANDLE readThread_ = INVALID_HANDLE_VALUE;  // 读取线程句柄
    MessageCallback messageCallback_;           // 消息回调
};

/**
 * IPC服务器类
 * 
 * 用于接受IPC客户端连接
 * 支持命名管道通信
 */
class IPCServer {
public:
    /**
     * 构造函数
     */
    IPCServer();
    
    /**
     * 析构函数
     */
    ~IPCServer();
    
    /**
     * 启动服务器
     * @param pipeName 管道名称
     * @return 是否启动成功
     */
    bool start(const std::string& pipeName = "\\\\.\\pipe\\aether_ui");
    
    /**
     * 停止服务器
     */
    void stop();
    
    /**
     * 检查服务器是否运行中
     * @return 如果运行中返回true
     */
    bool isRunning() const;
    
    /**
     * 设置消息回调
     * @param callback 回调函数
     */
    void setMessageCallback(MessageCallback callback) { messageCallback_ = std::move(callback); }
    
    /**
     * 广播消息给所有连接的客户端
     * @param message 消息字符串
     * @return 是否成功
     */
    bool broadcast(const std::string& message);
    
private:
    /**
     * 服务器线程函数
     * @param param 线程参数
     * @return 线程退出码
     */
    static DWORD WINAPI serverThread(LPVOID param);
    
    /**
     * 服务器主循环
     */
    void serverLoop();
    
    std::string pipeName_;                    // 管道名称
    HANDLE pipeHandle_ = INVALID_HANDLE_VALUE;  // 管道句柄
    bool running_ = false;                    // 运行状态
    HANDLE serverThreadHandle_ = INVALID_HANDLE_VALUE;  // 服务器线程句柄
    MessageCallback messageCallback_;         // 消息回调
};

}
