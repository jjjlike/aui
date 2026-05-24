cmake_minimum_required(VERSION 3.15)
project(Aether VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" OFF)
option(BUILD_SERVER "Build RPC server" ON)
option(BUILD_APP "Build desktop app" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(AETHER_SOURCES
    src/ComponentStorage.cpp
    src/PropertySystem.cpp
    src/LayoutEngine.cpp
    src/QuadTree.cpp
    src/EventDispatcher.cpp
    src/StateManager.cpp
    src/Snapshot.cpp
    src/TestController.cpp
    src/LogicLayer.cpp
)

if(WIN32)
    list(APPEND AETHER_SOURCES
        src/IPCClient.cpp
        src/JSONParser.cpp
        src/Direct2DRenderer.cpp
        src/AetherApplication.cpp
        src/RPCServer.cpp
    )
endif()

add_library(aether_logic STATIC ${AETHER_SOURCES})
target_include_directories(aether_logic PUBLIC ${CMAKE_SOURCE_DIR}/include)
if(MSVC)
    # 添加 UTF-8 支持，确保中文注释正常编译
    target_compile_options(aether_logic PRIVATE /W4 /utf-8)
    if(WIN32)
        target_link_libraries(aether_logic ws2_32 d2d1 dwrite)
    endif()
endif()

add_library(aether::logic ALIAS aether_logic)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_SERVER AND WIN32)
    add_executable(aether_logic_server server/main.cpp)
    target_link_libraries(aether_logic_server PRIVATE aether_logic)
    if(MSVC)
        target_compile_options(aether_logic_server PRIVATE /W4 /utf-8)
        target_link_libraries(aether_logic_server PRIVATE ws2_32)
    endif()
endif()

if(BUILD_APP AND WIN32)
    add_executable(aether_app WIN32 app/main.cpp)
    target_link_libraries(aether_app PRIVATE aether_logic)
    if(MSVC)
        target_compile_options(aether_app PRIVATE /W4 /utf-8)
        target_link_libraries(aether_app PRIVATE ws2_32 d2d1 dwrite)
    endif()
endif()

install(TARGETS aether_logic
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION include)
install(DIRECTORY include/aether DESTINATION include)
