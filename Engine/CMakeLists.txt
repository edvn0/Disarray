cmake_minimum_required(VERSION 3.25)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

find_package(Vulkan REQUIRED)


add_library(${PROJECT_NAME} STATIC
        ${THIRD_PARTY}/imgui/backends/imgui_impl_glfw.cpp
        ${THIRD_PARTY}/imgui/backends/imgui_impl_vulkan.cpp
        src/core/App.cpp
        src/core/Window.cpp
        src/core/Log.cpp
        src/core/DataBuffer.cpp
        src/graphics/Device.cpp
        src/graphics/Swapchain.cpp
        src/graphics/Instance.cpp
        src/graphics/Shader.cpp
        src/graphics/Image.cpp
        src/graphics/PushConstantLayout.cpp
        src/graphics/Texture.cpp
        src/graphics/PhysicalDevice.cpp
        src/graphics/QueueFamilyIndex.cpp
        src/graphics/CommandExecutor.cpp
        src/graphics/ImageLoader.cpp
        src/graphics/RenderPass.cpp
        src/graphics/Renderer.cpp
        src/graphics/VertexBuffer.cpp
        src/graphics/IndexBuffer.cpp
        src/graphics/Mesh.cpp
        src/graphics/ModelLoader.cpp
        src/graphics/Framebuffer.cpp
        src/graphics/Pipeline.cpp
        src/graphics/PipelineCache.cpp
        src/glfw/GLFWClock.cpp
        src/imgui/InterfaceLayer.cpp
        src/imgui/ImGuiBackends.cpp
        src/vulkan/Device.cpp
        src/vulkan/Allocator.cpp
        src/vulkan/MemoryAllocator.cpp
        src/vulkan/ImageUtilities.cpp
        src/vulkan/CleanupAwaiter.cpp
        src/vulkan/Window.cpp
        src/vulkan/Surface.cpp
        src/vulkan/Swapchain.cpp
        src/vulkan/ExtensionSupport.cpp
        src/vulkan/Instance.cpp
        src/vulkan/Shader.cpp
        src/vulkan/Pipeline.cpp
        src/vulkan/Image.cpp
        src/vulkan/Texture.cpp
        src/vulkan/CommandExecutor.cpp
        src/vulkan/RenderPass.cpp
        src/vulkan/VertexBuffer.cpp
        src/vulkan/IndexBuffer.cpp
        src/vulkan/Renderer.cpp
        src/vulkan/Mesh.cpp
        src/vulkan/Framebuffer.cpp
        src/vulkan/PhysicalDevice.cpp
        src/vulkan/RenderBatchFor.cpp
        src/vulkan/QueueFamilyIndex.cpp
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include . ${THIRD_PARTY}/glm ${THIRD_PARTY}/tinyobjloader ${THIRD_PARTY}/stb ${THIRD_PARTY}/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator OpenMP::OpenMP_CXX imgui)

if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif ()