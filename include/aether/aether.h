#pragma once

/**
 * @file aether.h
 * @brief Aether UI框架主头文件
 * 
 * Aether是一个现代化的UI框架，专注于自动化测试和声明式UI。
 * 本头文件包含了整个框架的所有公共接口。
 * 
 * 主要特性：
 * - Flexbox布局系统
 * - 组件化架构
 * - 事件系统
 * - 快照测试
 * - 语义化断言
 * - RPC远程控制
 * 
 * @version 1.0.0
 */

#define AETHER_VERSION "1.0.0"

// 基础类型定义
#include "types.h"

// 属性系统
#include "property_id.h"

// 组件存储
#include "ComponentStorage.h"

// 布局引擎
#include "LayoutEngine.h"

// 空间索引（四叉树）
#include "QuadTree.h"

// 事件系统
#include "EventDispatcher.h"

// 状态管理
#include "StateManager.h"

// 快照系统
#include "Snapshot.h"

// 测试控制器
#include "TestController.h"

// 逻辑层（统一接口）
#include "LogicLayer.h"

/**
 * @mainpage Aether UI 框架文档
 * 
 * @section intro_sec 介绍
 * 
 * Aether是一个高性能的UI框架，专为自动化测试和现代应用程序设计。
 * 
 * @section features_sec 主要特性
 * 
 * - @subpage layout_page "Flexbox布局"
 * - @subpage component_page "组件系统"
 * - @subpage event_page "事件处理"
 * - @subpage test_page "自动化测试"
 * - @subpage rpc_page "远程控制"
 * 
 * @section usage_sec 快速开始
 * 
 * @code
 * #include "aether/aether.h"
 * 
 * using namespace aether;
 * 
 * int main() {
 *     LogicLayer logic;
 *     
 *     // 创建组件
 *     auto button = logic.createComponent(ComponentType::Button);
 *     logic.setProperty(button, PropertyId::Text, "Click Me");
 *     
 *     // 运行
 *     logic.runFrame();
 *     
 *     return 0;
 * }
 * @endcode
 */
