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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/panels/ScenePanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/panels/ExecutionStatisticsPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ClientLayer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ClientLayer.hpp
        PARENT_SCOPE)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/panels/ScenePanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/panels/ExecutionStatisticsPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ClientLayer.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine argparse::argparse magic_enum::magic_enum thread-pool EnTT::EnTT fmt::fmt)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable -Wno-sign-compare)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_package(TBB REQUIRED)

    # VMA, STB ignores - -Wno-nullability-completeness -Wno-unused-variable
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable -Wno-sign-compare)
    target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # VMA, STB ignores - /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201
    # [[noreturn]] - /wd4702
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201 /wd4702 /wd4267)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_WINDOWS)
endif ()

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()

include("${CMAKE_SOURCE_DIR}/cmake/compile_shader.cmake")
compile_shaders()

include("${CMAKE_SOURCE_DIR}/cmake/copy_directory.cmake")
copy_directory_to_binary(Assets "Models;Shaders;Textures;Icons")

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()

if (${Vulkan_glslangValidator_FOUND})
    add_dependencies(${PROJECT_NAME} shaders)
endif ()
