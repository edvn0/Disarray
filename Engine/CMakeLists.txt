cmake_minimum_required(VERSION 3.22)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/App.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/DataBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Layer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ReferenceCounted.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/GLFWClock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/CommandExecutor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ImageLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/IndexBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ModelLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PhysicalDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PipelineCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PushConstantLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/QueueFamilyIndex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/RenderPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Swapchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/VertexBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/ImGuiBackends.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/InterfaceLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/CleanupAwaiter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/CommandExecutor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/ExtensionSupport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/ImageUtilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/IndexBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/MemoryAllocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/PhysicalDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/QueueFamilyIndex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/RenderBatchFor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/RenderPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Surface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Swapchain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/UI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Verify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/VertexBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/DebugConfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/DebugMarker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Disarray.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/AllocatorConfigurator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/App.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/CleanupAwaiter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Clock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/DataBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Ensure.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Entry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Layer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Log.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/UniquelyIdentifiable.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/UsageBadge.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/Window.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Forward.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/CommandExecutor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Framebuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Image.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ImageLoader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ImageProperties.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/IndexBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Instance.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Mesh.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/ModelLoader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/PhysicalDevice.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/PipelineCache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/PushConstantLayout.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/QueueFamilyIndex.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/RenderPass.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Surface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Swapchain.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/Texture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/graphics/VertexBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/InterfaceLayer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/UI.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/CommandExecutor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/ExtensionSupport.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Framebuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Image.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/ImageUtilities.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/IndexBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Instance.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/MemoryAllocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Mesh.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/PhysicalDevice.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/PropertySupplier.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/QueueFamilyIndex.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/RenderPass.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Surface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Swapchain.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/SwapchainUtilities.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Texture.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Verify.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/VertexBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan/Window.hpp
    PARENT_SCOPE)

add_library(${PROJECT_NAME} STATIC
    ${THIRD_PARTY}/imgui/backends/imgui_impl_glfw.cpp
    ${THIRD_PARTY}/imgui/backends/imgui_impl_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/App.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Layer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ReferenceCounted.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/UniformBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/IndexBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/ModelLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/Pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/PipelineCache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/GLFWClock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/glfw/GLFWInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/InterfaceLayer.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/DebugConfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/DebugMarker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/BaseBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/UniformBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Structures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/QueueFamilyIndex.cpp)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} PUBLIC include . ${THIRD_PARTY}/glm ${THIRD_PARTY}/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator imgui tinyobjloader stb_image thread-pool fmt::fmt)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/DisarrayPCH.hpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    find_package(TBB REQUIRED)

    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable)
    target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_package(TBB REQUIRED)

    # VMA, STB ignores - -Wno-nullability-completeness -Wno-unused-variable
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable)
    target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # VMA, STB ignores - /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201
    # [[noreturn]] - /wd4702
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201 /wd4702)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_WINDOWS)
endif()

if (DISARRAY_LOG_ALLOCATIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_ALLOCATIONS)
endif()

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
endif()
