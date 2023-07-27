cmake_minimum_required(VERSION 3.25)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)


add_library(${PROJECT_NAME} STATIC 
    src/core/app.cpp 
    src/graphics/device.cpp 
    src/core/window.cpp 
    src/core/log.cpp
    src/glfw/GLFWClock.cpp
    src/vulkan/device.cpp 
    src/vulkan/window.cpp 
    src/vulkan/surface.cpp
    src/vulkan/instance.cpp
    )
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include .)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif()