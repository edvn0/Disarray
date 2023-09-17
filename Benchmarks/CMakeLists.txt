cmake_minimum_required(VERSION 3.22)

project(DisarrayBenchmarks CXX)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(ThirdParty/benchmark)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine benchmark::benchmark benchmark::benchmark_main)
default_compile_flags()
