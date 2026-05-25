// Copyright (c) 2026 jaether
// jaether is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.


#pragma once

#include "aether/EventDispatcher.h"
#include "aether/JSONParser.h"
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <unordered_map>
#include <windows.h>

namespace jaether {

/**
 * RPC处理器函数类型
 * 
 * @param params 参数JSON
 * @return 结果JSON
 */
using RPCHandler = std::function<JJSONValue(const JJSONValue& params)>;

/**
 * RPC服务器类
 * 
 * 提供基于Windows命名管道的远程过程调用服务
 * 用于测试和自动化控制
 */
class JRPCServer {
public:
    /**
     * 构造函数
     */
    JRPCServer();
    
    /**
     * 析构函数
     */
    ~JRPCServer();
    
    /**
     * 启动RPC服务器
     * 
     * @param pipeName 管道名称
     * @return 成功返回true
     */
    bool start(const std::string& pipeName);
    
    /**
     * 关闭RPC服务器
     */
    void shutdown();
    
    /**
     * 注册RPC处理函数
     * 
     * @param name 方法名
     * @param handler 处理函数
     */
    void registerHandler(const std::string& name, RPCHandler handler);
    
private:
    /**
     * 服务器主循环
     */
    void serverLoop();
    
    /**
     * 处理单个客户端连接
     * 
     * @param hPipe 管道句柄
     */
    void handleClient(HANDLE hPipe);
    
    /**
     * 调用处理函数
     * 
     * @param method 方法名
     * @param params 参数
     * @return 结果
     */
    JJSONValue invokeHandler(const std::string& method, const JJSONValue& params);
    
    std::string pipeName_;                    // 管道名称
    HANDLE hPipe_ = INVALID_HANDLE_VALUE;     // 管道句柄
    bool running_ = false;                    // 运行状态
    std::thread serverThread_;                // 服务器线程
    std::unordered_map<std::string, RPCHandler> handlers_;  // 处理函数表
};

}
