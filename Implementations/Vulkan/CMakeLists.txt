cmake_minimum_required(VERSION 3.22)

project(vk CXX)
set(SOURCES
        include/vulkan/Allocator.hpp
        include/vulkan/BaseBuffer.hpp
        include/vulkan/CommandExecutor.hpp
        include/vulkan/Config.hpp
        include/vulkan/DebugMarker.hpp
        include/vulkan/Device.hpp
        include/vulkan/exceptions/VulkanExceptions.hpp
        include/vulkan/ExtensionSupport.hpp
        include/vulkan/Framebuffer.hpp
        include/vulkan/GraphicsResource.hpp
        include/vulkan/Image.hpp
        include/vulkan/ImageUtilities.hpp
        include/vulkan/IncludeDirectoryIncluder.hpp
        include/vulkan/IndexBuffer.hpp
        include/vulkan/Instance.hpp
        include/vulkan/Material.hpp
        include/vulkan/MemoryAllocator.hpp
        include/vulkan/Mesh.hpp
        include/vulkan/MeshMaterial.hpp
        include/vulkan/PhysicalDevice.hpp
        include/vulkan/Pipeline.hpp
        include/vulkan/PropertySupplier.hpp
        include/vulkan/QueueFamilyIndex.hpp
        include/vulkan/Renderer.hpp
        include/vulkan/RenderPass.hpp
        include/vulkan/Shader.hpp
        include/vulkan/SingleShader.hpp
        include/vulkan/StorageBuffer.hpp
        include/vulkan/StaticMesh.hpp
        include/vulkan/Structures.hpp
        include/vulkan/Surface.hpp
        include/vulkan/Swapchain.hpp
        include/vulkan/SwapchainUtilities.hpp
        include/vulkan/Texture.hpp
        include/vulkan/UniformBuffer.hpp
        include/vulkan/Verify.hpp
        include/vulkan/VertexBuffer.hpp
        include/vulkan/Window.hpp
        src/vulkan/Allocator.cpp
        src/vulkan/BaseBuffer.cpp
        src/vulkan/CleanupAwaiter.cpp
        src/vulkan/CommandExecutor.cpp
        src/vulkan/DebugConfigurator.cpp
        src/vulkan/DebugMarker.cpp
        src/vulkan/Device.cpp
        src/vulkan/ExtensionSupport.cpp
        src/vulkan/Framebuffer.cpp
        src/vulkan/GlslangCompiler.cpp
        src/vulkan/GraphicsResource.cpp
        src/vulkan/Image.cpp
        src/vulkan/ImageUtilities.cpp
        src/vulkan/IncludeDirectoryIncluder.cpp
        src/vulkan/IndexBuffer.cpp
        src/vulkan/Instance.cpp
        src/vulkan/Material.cpp
        src/vulkan/MemoryAllocator.cpp
        src/vulkan/Mesh.cpp
        src/vulkan/MeshMaterial.cpp
        src/vulkan/PhysicalDevice.cpp
        src/vulkan/Pipeline.cpp
        src/vulkan/QueueFamilyIndex.cpp
        src/vulkan/RenderBatchImplementation.cpp
        src/vulkan/renderer/GraphicsRenderer.cpp
        src/vulkan/renderer/Renderer.cpp
        src/vulkan/RenderPass.cpp
        src/vulkan/Shader.cpp
        src/vulkan/ShaderReflectionData.cpp
        src/vulkan/SingleShader.cpp
        src/vulkan/StorageBuffer.cpp
        src/vulkan/StaticMesh.cpp
        src/vulkan/Structures.cpp
        src/vulkan/Surface.cpp
        src/vulkan/Swapchain.cpp
        src/vulkan/TextRenderer.cpp
        src/vulkan/Texture.cpp
        src/vulkan/UI.cpp
        src/vulkan/UniformBuffer.cpp
        src/vulkan/Verify.cpp
        src/vulkan/VertexBuffer.cpp
        src/vulkan/Window.cpp
)

if (DISARRAY_HAS_GLSLC)
    list(APPEND SOURCES "src/vulkan/GlslcCompiler.cpp")
else ()
    list(APPEND SOURCES "src/vulkan/GlslangCompiler.cpp" "src/vulkan/IncludeDirectoryIncluder.cpp")
endif ()

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(Implementations::Vulkan ALIAS ${PROJECT_NAME})
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE Disarray::Engine
        glfw
        assimp::assimp
        GPUOpen::VulkanMemoryAllocator
        thread-pool
        Vulkan::Vulkan
        imgui
        fmt::fmt
        magic_enum::magic_enum
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-reflect
        glslang
        glslang-default-resource-limits
        SPIRV
        freetype
        ktx
)

target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE inline)
default_compile_flags()
