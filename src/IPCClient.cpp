// JIPCClient.cpp
// IPC通信模块 - 进程间通信（Windows命名管道）
//
// 功能:
// - 客户端连接到命名管道
// - 服务器创建命名管道
// - 消息发送和接收
// - 异步读取支持

#include "aether/IPCClient.h"
#include <iostream>

namespace jaether {

// IPC客户端构造函数
JIPCClient::JIPCClient() {
}

// IPC客户端析构函数
JIPCClient::~JIPCClient() {
    disconnect();
}

// 连接到命名管道
// 参数: pipeName - 管道名称
// 返回值: 成功返回true
bool JIPCClient::connect(const std::string& pipeName) {
    if (connected_) {
        return true;
    }
    
    // 打开命名管道
    pipeHandle_ = CreateFileA(
        pipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    
    if (pipeHandle_ == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect to pipe: " << GetLastError() << std::endl;
        return false;
    }
    
    // 设置管道为消息模式
    DWORD mode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(pipeHandle_, &mode, NULL, NULL)) {
        std::cerr << "SetNamedPipeHandleState failed: " << GetLastError() << std::endl;
        CloseHandle(pipeHandle_);
        pipeHandle_ = INVALID_HANDLE_VALUE;
        return false;
    }
    
    connected_ = true;
    return true;
}

// 断开连接
void JIPCClient::disconnect() {
    stopAsyncRead();
    
    if (pipeHandle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipeHandle_);
        pipeHandle_ = INVALID_HANDLE_VALUE;
    }
    connected_ = false;
}

// 检查是否已连接
bool JIPCClient::isConnected() const {
    return connected_;
}

// 发送消息
// 参数: message - 消息字符串
// 返回值: 成功返回true
bool JIPCClient::sendMessage(const std::string& message) {
    if (!connected_ || pipeHandle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesWritten;
    BOOL result = WriteFile(
        pipeHandle_,
        message.c_str(),
        static_cast<DWORD>(message.length()),
        &bytesWritten,
        NULL
    );
    
    if (!result || bytesWritten != message.length()) {
        std::cerr << "WriteFile failed: " << GetLastError() << std::endl;
        return false;
    }
    
    return true;
}

// 接收消息
// 返回值: 接收到的消息字符串
std::string JIPCClient::receiveMessage() {
    if (!connected_ || pipeHandle_ == INVALID_HANDLE_VALUE) {
        return "";
    }
    
    char buffer[8192] = {0};
    DWORD bytesRead;
    
    BOOL result = ReadFile(
        pipeHandle_,
        buffer,
        sizeof(buffer) - 1,
        &bytesRead,
        NULL
    );
    
    if (!result) {
        std::cerr << "ReadFile failed: " << GetLastError() << std::endl;
        return "";
    }
    
    return std::string(buffer, bytesRead);
}

// 开始异步读取
void JIPCClient::startAsyncRead() {
    if (reading_) {
        return;
    }
    
    reading_ = true;
    readThread_ = CreateThread(NULL, 0, asyncReadThread, this, 0, NULL);
}

// 停止异步读取
void JIPCClient::stopAsyncRead() {
    reading_ = false;
    
    if (readThread_ != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(readThread_, INFINITE);
        CloseHandle(readThread_);
        readThread_ = INVALID_HANDLE_VALUE;
    }
}

// 异步读取线程（静态）
// 参数: param - IPCClient指针
// 返回值: 线程退出码
DWORD WINAPI JIPCClient::asyncReadThread(LPVOID param) {
    JIPCClient* client = static_cast<JIPCClient*>(param);
    client->readLoop();
    return 0;
}

// 读取循环
void JIPCClient::readLoop() {
    while (reading_ && connected_) {
        std::string message = receiveMessage();
        if (!message.empty() && messageCallback_) {
            messageCallback_(message);
        }
    }
}

// IPC服务器构造函数
IPCServer::IPCServer() {
}

// IPC服务器析构函数
IPCServer::~IPCServer() {
    stop();
}

// 启动服务器
// 参数: pipeName - 管道名称
// 返回值: 成功返回true
bool IPCServer::start(const std::string& pipeName) {
    pipeName_ = pipeName;
    
    // 创建命名管道
    pipeHandle_ = CreateNamedPipeA(
        pipeName_.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        8192,
        8192,
        0,
        NULL
    );
    
    if (pipeHandle_ == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateNamedPipe failed: " << GetLastError() << std::endl;
        return false;
    }
    
    running_ = true;
    serverThreadHandle_ = CreateThread(NULL, 0, serverThread, this, 0, NULL);
    
    return true;
}

// 停止服务器
void IPCServer::stop() {
    running_ = false;
    
    if (serverThreadHandle_ != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(serverThreadHandle_, INFINITE);
        CloseHandle(serverThreadHandle_);
        serverThreadHandle_ = INVALID_HANDLE_VALUE;
    }
    
    if (pipeHandle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipeHandle_);
        pipeHandle_ = INVALID_HANDLE_VALUE;
    }
}

// 检查服务器是否运行
bool IPCServer::isRunning() const {
    return running_;
}

// 广播消息
// 参数: message - 消息字符串
// 返回值: 成功返回true
bool IPCServer::broadcast(const std::string& message) {
    if (!running_ || pipeHandle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesWritten;
    BOOL result = WriteFile(
        pipeHandle_,
        message.c_str(),
        static_cast<DWORD>(message.length()),
        &bytesWritten,
        NULL
    );
    
    return result && bytesWritten == message.length();
}

// 服务器线程（静态）
// 参数: param - IPCServer指针
// 返回值: 线程退出码
DWORD WINAPI IPCServer::serverThread(LPVOID param) {
    IPCServer* server = static_cast<IPCServer*>(param);
    server->serverLoop();
    return 0;
}

// 服务器循环
void IPCServer::serverLoop() {
    while (running_) {
        // 等待客户端连接
        BOOL connected = ConnectNamedPipe(pipeHandle_, NULL);
        if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
            if (GetLastError() != ERROR_IO_PENDING) {
                std::cerr << "ConnectNamedPipe failed: " << GetLastError() << std::endl;
                break;
            }
        }
        
        // 读取客户端消息
        char buffer[8192] = {0};
        DWORD bytesRead;
        BOOL result = ReadFile(
            pipeHandle_,
            buffer,
            sizeof(buffer) - 1,
            &bytesRead,
            NULL
        );
        
        if (result && bytesRead > 0) {
            if (messageCallback_) {
                messageCallback_(std::string(buffer, bytesRead));
            }
        }
        
        // 断开客户端连接
        DisconnectNamedPipe(pipeHandle_);
    }
}

} // namespace jaether
