cmake_minimum_required(VERSION 3.25)

project(App CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/argparse)

set(APP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ClientLayer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ClientLayer.hpp
        PARENT_SCOPE)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ClientLayer.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine argparse::argparse)

include("${CMAKE_SOURCE_DIR}/cmake/compile_shader.cmake")
compile_shaders()

include("${CMAKE_SOURCE_DIR}/cmake/copy_directory.cmake")
copy_directory_to_binary(Assets "Models;Shaders;Textures")

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()

add_dependencies(${PROJECT_NAME} shaders)
