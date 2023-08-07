cmake_minimum_required(VERSION 3.22)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE ALL_HEADERS "include/*.hpp")
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")

list(APPEND ${ALL_SOURCES} ${ALL_HEADERS})
set(ENGINE_SOURCES ${ALL_SOURCES} PARENT_SCOPE)

add_library(${PROJECT_NAME} STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Disarray.hpp
        ${THIRD_PARTY}/imgui/backends/imgui_impl_glfw.cpp
        ${THIRD_PARTY}/imgui/backends/imgui_impl_vulkan.cpp
        ${ALL_SOURCES})

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include . ${THIRD_PARTY}/glm ${THIRD_PARTY}/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator imgui tinyobjloader stb_image thread-pool EnTT::EnTT fmt::fmt)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/DisarrayPCH.hpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    find_package(TBB REQUIRED)

    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable)
    target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_package(TBB REQUIRED)

    # VMA, STB ignores - -Wno-nullability-completeness -Wno-unused-variable
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable)
    target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # VMA, STB ignores - /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201
    # [[noreturn]] - /wd4702
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201 /wd4702)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_WINDOWS)
endif ()

if (DISARRAY_LOG_ALLOCATIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_ALLOCATIONS)
endif ()

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()
