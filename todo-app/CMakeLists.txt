# todo-app/CMakeLists.txt
# 待办列表应用 - 独立工程
#
# 功能: 完整的待办列表应用程序
# 依赖: Aether UI Engine (通过子目录或外部库)
#
# 使用说明:
#   mkdir build && cd build
#   cmake -G "Visual Studio 17 2022" -A x64 ..
#   cmake --build . --config Debug

cmake_minimum_required(VERSION 3.15)
project(TodoApp VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置Aether源目录（父目录）
set(AETHER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/.." CACHE PATH "Path to Aether UI Engine")

# 验证Aether目录
if(NOT EXISTS "${AETHER_SOURCE_DIR}/include/aether/LogicLayer.h")
    message(FATAL_ERROR "Cannot find Aether headers in ${AETHER_SOURCE_DIR}")
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${AETHER_SOURCE_DIR}/include
)

# Aether库源文件
set(AETHER_SOURCES
    ${AETHER_SOURCE_DIR}/src/ComponentStorage.cpp
    ${AETHER_SOURCE_DIR}/src/PropertySystem.cpp
    ${AETHER_SOURCE_DIR}/src/LayoutEngine.cpp
    ${AETHER_SOURCE_DIR}/src/QuadTree.cpp
    ${AETHER_SOURCE_DIR}/src/EventDispatcher.cpp
    ${AETHER_SOURCE_DIR}/src/StateManager.cpp
    ${AETHER_SOURCE_DIR}/src/Snapshot.cpp
    ${AETHER_SOURCE_DIR}/src/TestController.cpp
    ${AETHER_SOURCE_DIR}/src/LogicLayer.cpp
    ${AETHER_SOURCE_DIR}/src/JSONParser.cpp
    ${AETHER_SOURCE_DIR}/src/Direct2DRenderer.cpp
    ${AETHER_SOURCE_DIR}/src/IPCClient.cpp
    ${AETHER_SOURCE_DIR}/src/RPCServer.cpp
)

# TodoApp核心源文件（不含main）
set(TODOAPP_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/TodoApp.cpp
)

# 待办应用源文件（含main）
set(TODOAPP_SOURCES
    ${TODOAPP_CORE_SOURCES}
    ${CMAKE_SOURCE_DIR}/main.cpp
)

# 测试源文件
set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/TodoAppTest.cpp
)

# 创建待办应用可执行文件
add_executable(todo_app WIN32 ${AETHER_SOURCES} ${TODOAPP_SOURCES})

# 设置输出名称
set_target_properties(todo_app PROPERTIES
    OUTPUT_NAME "TodoApp"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Windows特定链接库
if(WIN32)
    target_link_libraries(todo_app PRIVATE ws2_32 d2d1 dwrite ole32)
endif()

# 编译选项
if(MSVC)
    target_compile_options(todo_app PRIVATE /W4 /utf-8)
endif()

# 可选: 构建测试
option(BUILD_TESTS "Build TodoApp tests" ON)

if(BUILD_TESTS)
    # 获取GoogleTest
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    # 创建测试可执行文件
    add_executable(todoapp_tests ${AETHER_SOURCES} ${TODOAPP_CORE_SOURCES} ${TEST_SOURCES})
    
    # 设置输出目录
    set_target_properties(todoapp_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )
    
    target_link_libraries(todoapp_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock)
    
    if(WIN32)
        target_link_libraries(todoapp_tests PRIVATE ws2_32 d2d1 dwrite ole32)
    endif()
    
    if(MSVC)
        target_compile_options(todoapp_tests PRIVATE /W4 /utf-8)
    endif()
    
    # 发现并注册测试
    include(GoogleTest)
    gtest_discover_tests(todoapp_tests)
endif()

# 打印配置信息
message(STATUS "")
message(STATUS "=====================================")
message(STATUS "Todo App Configuration Summary")
message(STATUS "=====================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Aether Source: ${AETHER_SOURCE_DIR}")
message(STATUS "")
