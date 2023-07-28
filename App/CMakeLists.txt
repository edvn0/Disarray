cmake_minimum_required(VERSION 3.25)

project(App CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED COMPONENTS glslc)

add_executable(${PROJECT_NAME} src/Application.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine)

include("${CMAKE_SOURCE_DIR}/cmake/compile_shader.cmake")
compile_shaders()

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif()

add_dependencies(${PROJECT_NAME} shaders)