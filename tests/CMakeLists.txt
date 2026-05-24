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

include_directories(${CMAKE_SOURCE_DIR}/include)

set(TEST_SOURCES
    ComponentStorage_test.cpp
    PropertySystem_test.cpp
    LayoutEngine_test.cpp
    QuadTree_test.cpp
    EventDispatcher_test.cpp
    EventRecording_test.cpp
    StateManager_test.cpp
    Snapshot_test.cpp
    TestController_test.cpp
    LogicLayer_test.cpp
    JSONParser_test.cpp
    Integration_test.cpp
)

add_executable(aether_tests ${TEST_SOURCES})
target_link_libraries(aether_tests PRIVATE aether_logic GTest::gtest GTest::gtest_main GTest::gmock)
target_compile_options(aether_tests PRIVATE /W4 /utf-8)

include(GoogleTest)
gtest_discover_tests(aether_tests)
