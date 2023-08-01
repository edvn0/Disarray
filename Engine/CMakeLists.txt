cmake_minimum_required(VERSION 3.25)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

set(ENGINE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/App.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/DataBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Swapchain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Instance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PushConstantLayout.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PhysicalDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/QueueFamilyIndex.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/CommandExecutor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ImageLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/RenderPass.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/VertexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/IndexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ModelLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Framebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PipelineCache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/GLFWClock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/InterfaceLayer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/ImGuiBackends.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Allocator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/MemoryAllocator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/ImageUtilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/CleanupAwaiter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/UI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Swapchain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/ExtensionSupport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Verify.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Instance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/CommandExecutor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/RenderPass.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/VertexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/IndexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Framebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/PhysicalDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/RenderBatchFor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/QueueFamilyIndex.cpp
        PARENT_SCOPE)

add_library(${PROJECT_NAME} STATIC
        ${THIRD_PARTY}/imgui/backends/imgui_impl_glfw.cpp
        ${THIRD_PARTY}/imgui/backends/imgui_impl_vulkan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/App.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/DataBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Swapchain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Instance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PushConstantLayout.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PhysicalDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/QueueFamilyIndex.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/CommandExecutor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ImageLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/RenderPass.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/VertexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/IndexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ModelLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Framebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PipelineCache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/GLFWClock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/InterfaceLayer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/ImGuiBackends.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Allocator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/MemoryAllocator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/ImageUtilities.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/CleanupAwaiter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/UI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Swapchain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/ExtensionSupport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Verify.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Instance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/CommandExecutor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/RenderPass.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/VertexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/IndexBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Mesh.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Framebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/PhysicalDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/RenderBatchFor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/QueueFamilyIndex.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include . ${THIRD_PARTY}/glm ${THIRD_PARTY}/tinyobjloader ${THIRD_PARTY}/stb ${THIRD_PARTY}/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator OpenMP::OpenMP_CXX imgui)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()