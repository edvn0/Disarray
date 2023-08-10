cmake_minimum_required(VERSION 3.22)

project(App CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/argparse)

set(APP_SOURCES
        include/panels/ScenePanel.hpp
        src/panels/ScenePanel.cpp
        include/panels/ExecutionStatisticsPanel.hpp
        src/panels/ExecutionStatisticsPanel.cpp
        include/panels/DirectoryContentPanel.hpp
        src/panels/DirectoryContentPanel.cpp
        include/ClientLayer.hpp
        src/ClientLayer.cpp)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp ${APP_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine Vulkan::Vulkan argparse::argparse magic_enum::magic_enum thread-pool EnTT::EnTT fmt::fmt)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS)

default_compile_flags()
if (DISARRAY_COMPILER STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4267)
endif ()

include("${CMAKE_SOURCE_DIR}/cmake/compile_shader.cmake")
compile_shaders()

include("${CMAKE_SOURCE_DIR}/cmake/copy_directory.cmake")
copy_directory_to_binary(Assets "Models;Shaders;Textures;Icons")

if (${Vulkan_glslangValidator_FOUND})
    add_dependencies(${PROJECT_NAME} shaders)
endif ()
