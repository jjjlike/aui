find_package(benchmark QUIET)

if(NOT benchmark_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.0
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googlebenchmark)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(BENCHMARK_SOURCES
    benchmark_main.cpp
    LayoutBench.cpp
    QuadTreeBench.cpp
    ComponentStorageBench.cpp
)

add_executable(aether_benchmarks ${BENCHMARK_SOURCES})
target_link_libraries(aether_benchmarks PRIVATE aether_logic benchmark::benchmark)
target_compile_options(aether_benchmarks PRIVATE /W4 /permissf-)
