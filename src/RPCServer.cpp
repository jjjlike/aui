// JRPCServer.cpp
// RPC服务器模块 - 提供远程过程调用服务
//
// 功能:
// - 命名管道服务器创建
// - 客户端连接处理
// - 请求解析和分发
// - 响应发送

#include "aether/RPCServer.h"
#include <iostream>
#include <windows.h>

namespace jaether {

// RPC服务器构造函数
JRPCServer::JRPCServer() {
}

// RPC服务器析构函数
JRPCServer::~JRPCServer() {
    shutdown();
}

// 启动RPC服务器
// 参数: pipeName - 管道名称
// 返回值: 成功返回true
bool JRPCServer::start(const std::string& pipeName) {
    pipeName_ = pipeName;

    // 创建管道名称
    std::wstring wPipeName = std::wstring(L"\\\\.\\pipe\\") + std::wstring(pipeName.begin(), pipeName.end());

    // 创建第一个管道实例
    hPipe_ = CreateNamedPipeW(
        wPipeName.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        4096,
        4096,
        0,
        NULL
    );

    if (hPipe_ == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create named pipe" << std::endl;
        return false;
    }

    running_ = true;

    // 启动服务器线程
    serverThread_ = std::thread(&JRPCServer::serverLoop, this);

    return true;
}

// 关闭RPC服务器
void JRPCServer::shutdown() {
    if (!running_) {
        return;
    }

    running_ = false;

    // 关闭管道
    if (hPipe_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe_);
        hPipe_ = INVALID_HANDLE_VALUE;
    }

    // 等待服务器线程结束
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
}

// 注册RPC处理函数
// 参数:
//   name - 方法名
//   handler - 处理函数
void JRPCServer::registerHandler(const std::string& name, RPCHandler handler) {
    handlers_[name] = std::move(handler);
}

// 服务器主循环
void JRPCServer::serverLoop() {
    while (running_) {
        // 等待客户端连接
        if (!ConnectNamedPipe(hPipe_, NULL)) {
            if (GetLastError() != ERROR_PIPE_CONNECTED) {
                std::cerr << "ConnectNamedPipe failed" << std::endl;
                break;
            }
        }

        // 处理客户端
        handleClient(hPipe_);

        // 断开连接并准备接受下一个连接
        DisconnectNamedPipe(hPipe_);
    }
}

// 处理单个客户端
// 参数: hPipe - 管道句柄
void JRPCServer::handleClient(HANDLE hPipe) {
    std::string request;
    char buffer[4096];
    DWORD bytesRead;

    // 读取请求
    while (true) {
        if (!ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            break;
        }
        request.append(buffer, bytesRead);

        if (bytesRead < sizeof(buffer)) {
            break;
        }
    }

    if (!request.empty()) {
        // 解析请求
        JJSONValue json = JJSONParser::parse(request);

        // 提取方法名和参数
        const auto& obj = std::get<JJSONObject>(json.value);
        const auto& methodVal = obj.at("method");
        const auto& paramsVal = obj.at("params");
        std::string method = std::get<std::string>(methodVal.value);
        JJSONValue params = paramsVal;

        // 调用处理函数
        JJSONValue result = invokeHandler(method, params);

        // 构建响应
        JJSONObject response;
        response["result"] = result;

        // 发送响应
        std::string responseStr = JJSONParser::stringify(JJSONValue(std::move(response)));

        DWORD bytesWritten;
        WriteFile(hPipe, responseStr.c_str(), static_cast<DWORD>(responseStr.size()), &bytesWritten, NULL);
    }
}

// 调用处理函数
// 参数:
//   method - 方法名
//   params - 参数
// 返回值: 结果
JJSONValue JRPCServer::invokeHandler(const std::string& method, const JJSONValue& params) {
    auto it = handlers_.find(method);
    if (it == handlers_.end()) {
        // 方法未找到
        JJSONObject error;
        error["error"] = JJSONValue(std::string("Method not found"));
        return JJSONValue(std::move(error));
    }

    try {
        // 调用处理函数
        return it->second(params);
    } catch (const std::exception& e) {
        // 发生错误
        JJSONObject error;
        error["error"] = JJSONValue(std::string(e.what()));
        return JJSONValue(std::move(error));
    }
}

} // namespace jaether
