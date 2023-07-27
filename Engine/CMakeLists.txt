cmake_minimum_required(VERSION 3.25)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

find_package(Vulkan REQUIRED)


add_library(${PROJECT_NAME} STATIC 
    src/core/App.cpp 
    src/core/Window.cpp 
    src/core/Log.cpp
    src/graphics/Device.cpp
    src/graphics/Swapchain.cpp
    src/graphics/Instance.cpp
    src/graphics/PhysicalDevice.cpp
    src/glfw/GLFWClock.cpp
    src/vulkan/Device.cpp 
    src/vulkan/Window.cpp 
    src/vulkan/Surface.cpp
    src/vulkan/Swapchain.cpp
    src/vulkan/ExtensionSupport.cpp
    src/vulkan/Instance.cpp
    src/vulkan/PhysicalDevice.cpp
    src/vulkan/QueueFamilyIndex.cpp
    )
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include . ${ThirdParty}/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif()