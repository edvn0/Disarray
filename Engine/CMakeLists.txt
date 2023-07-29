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
        src/core/DataBuffer.cpp
        src/graphics/Device.cpp
        src/graphics/Swapchain.cpp
        src/graphics/Instance.cpp
        src/graphics/Shader.cpp
        src/graphics/PhysicalDevice.cpp
        src/graphics/QueueFamilyIndex.cpp
        src/graphics/CommandExecutor.cpp
        src/graphics/RenderPass.cpp
        src/graphics/Renderer.cpp
        src/graphics/VertexBuffer.cpp
        src/graphics/IndexBuffer.cpp
        src/graphics/Mesh.cpp
        src/graphics/Framebuffer.cpp
        src/graphics/Pipeline.cpp
        src/graphics/PipelineCache.cpp
        src/glfw/GLFWClock.cpp
        src/vulkan/Device.cpp
        src/vulkan/Allocator.cpp
        src/vulkan/MemoryAllocator.cpp
        src/vulkan/CleanupAwaiter.cpp
        src/vulkan/Window.cpp
        src/vulkan/Surface.cpp
        src/vulkan/Swapchain.cpp
        src/vulkan/ExtensionSupport.cpp
        src/vulkan/Instance.cpp
        src/vulkan/Shader.cpp
        src/vulkan/Pipeline.cpp
        src/vulkan/CommandExecutor.cpp
        src/vulkan/RenderPass.cpp
        src/vulkan/VertexBuffer.cpp
        src/vulkan/IndexBuffer.cpp
        src/vulkan/Renderer.cpp
        src/vulkan/Mesh.cpp
        src/vulkan/Framebuffer.cpp
        src/vulkan/PhysicalDevice.cpp
        src/vulkan/QueueFamilyIndex.cpp
        )
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include . ${THIRD_PARTY}/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()