cmake_minimum_required(VERSION 3.22)

project(Engine CXX)


FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_SOURCES
        include/Forward.hpp
        include/graphics/BufferProperties.hpp
        include/graphics/RenderPass.hpp
        include/graphics/Renderer.hpp
        include/graphics/Pipeline.hpp
        include/graphics/IndexBuffer.hpp
        include/graphics/VertexBuffer.hpp
        include/graphics/CommandExecutor.hpp
        include/graphics/Device.hpp
        include/graphics/Material.hpp
        include/graphics/ImageProperties.hpp
        include/graphics/Surface.hpp
        include/graphics/Swapchain.hpp
        include/graphics/PhysicalDevice.hpp
        include/graphics/Shader.hpp
        include/graphics/PushConstantLayout.hpp
        include/graphics/Image.hpp
        include/graphics/ResourceCache.hpp
        include/graphics/Texture.hpp
        include/graphics/QueueFamilyIndex.hpp
        include/graphics/PipelineCache.hpp
        include/graphics/ImageLoader.hpp
        include/graphics/RenderBatch.hpp
        include/graphics/Mesh.hpp
        include/graphics/VertexTypes.hpp
        include/graphics/Framebuffer.hpp
        include/graphics/Instance.hpp
        include/graphics/ModelLoader.hpp
        include/graphics/TextureCache.hpp
        include/graphics/UniformBuffer.hpp
        include/ui/InterfaceLayer.hpp
        include/ui/UI.hpp
        include/DisarrayPCH.hpp
        include/scene/Component.hpp
        include/scene/Camera.hpp
        include/scene/Components.hpp
        include/scene/Entity.hpp
        include/scene/Serialiser.hpp
        include/scene/ComponentSerialisers.hpp
        include/scene/Scene.hpp
        include/core/ThreadPool.hpp
        include/core/FileWatcher.hpp
        include/core/App.hpp
        include/core/Hashes.hpp
        include/core/UsageBadge.hpp
        include/core/ReferenceCounted.hpp
        include/core/Window.hpp
        include/core/AllocatorConfigurator.hpp
        include/core/events/ApplicationEvent.hpp
        include/core/events/Event.hpp
        include/core/events/KeyEvent.hpp
        include/core/events/MouseEvent.hpp
        include/core/Types.hpp
        include/core/DebugConfigurator.hpp
        include/core/DisarrayObject.hpp
        include/core/Formatters.hpp
        include/core/Layer.hpp
        include/core/Clock.hpp
        include/core/KeyCode.hpp
        include/core/Log.hpp
        include/core/Tuple.hpp
        include/core/Platform.hpp
        include/core/CleanupAwaiter.hpp
        include/core/PointerDefinition.hpp
        include/core/DataBuffer.hpp
        include/core/Input.hpp
        include/core/Entry.hpp
        include/core/UniquelyIdentifiable.hpp
        include/core/MouseCode.hpp
        include/core/Panel.hpp
        include/core/PolymorphicCast.hpp
        include/core/Ensure.hpp
        include/core/Concepts.hpp
        include/util/BitCast.hpp
        include/util/FormattingUtilities.hpp
        include/vulkan/Config.hpp
        include/vulkan/RenderPass.hpp
        include/vulkan/Renderer.hpp
        include/vulkan/Pipeline.hpp
        include/vulkan/PropertySupplier.hpp
        include/vulkan/IndexBuffer.hpp
        include/vulkan/VertexBuffer.hpp
        include/vulkan/MemoryAllocator.hpp
        include/vulkan/ExtensionSupport.hpp
        include/vulkan/CommandExecutor.hpp
        include/vulkan/Allocator.hpp
        include/vulkan/Window.hpp
        include/vulkan/Device.hpp
        include/vulkan/Material.hpp
        include/vulkan/Surface.hpp
        include/vulkan/Swapchain.hpp
        include/vulkan/PhysicalDevice.hpp
        include/vulkan/BaseBuffer.hpp
        include/vulkan/Shader.hpp
        include/vulkan/SwapchainUtilities.hpp
        include/vulkan/ImageUtilities.hpp
        include/vulkan/Image.hpp
        include/vulkan/Texture.hpp
        include/vulkan/Verify.hpp
        include/vulkan/render_batch_implementation/RenderBatchImplementation.hpp
        include/vulkan/QueueFamilyIndex.hpp
        include/vulkan/Mesh.hpp
        include/vulkan/Framebuffer.hpp
        include/vulkan/Instance.hpp
        include/vulkan/Structures.hpp
        include/vulkan/UniformBuffer.hpp
        include/vulkan/DebugMarker.hpp
        Disarray.hpp
        src/graphics/Shader.cpp
        src/graphics/CommandExecutor.cpp
        src/graphics/RenderPass.cpp
        src/graphics/Instance.cpp
        src/graphics/PhysicalDevice.cpp
        src/graphics/Mesh.cpp
        src/graphics/ModelLoader.cpp
        src/graphics/PushConstantLayout.cpp
        src/graphics/Framebuffer.cpp
        src/graphics/VertexBuffer.cpp
        src/graphics/Swapchain.cpp
        src/graphics/QueueFamilyIndex.cpp
        src/graphics/UniformBuffer.cpp
        src/graphics/PipelineCache.cpp
        src/graphics/Renderer.cpp
        src/graphics/Pipeline.cpp
        src/graphics/Device.cpp
        src/graphics/Material.cpp
        src/graphics/ImageLoader.cpp
        src/graphics/IndexBuffer.cpp
        src/graphics/Texture.cpp
        src/graphics/Image.cpp
        src/glfw/GLFWInput.cpp
        src/glfw/GLFWClock.cpp
        src/scene/Components.cpp
        src/scene/Entity.cpp
        src/scene/Scene.cpp
        src/scene/Camera.cpp
        src/scene/ComponentSerialisers.cpp
        src/scene/ComponentDeserialisers.cpp
        src/scene/SerialisationTypeConversions.cpp
        src/core/DataBuffer.cpp
        src/core/Layer.cpp
        src/core/ReferenceCounted.cpp
        src/core/App.cpp
        src/core/FileWatcher.cpp
        src/core/Window.cpp
        src/core/Formatters.cpp
        src/core/Log.cpp
        src/vulkan/CleanupAwaiter.cpp
        src/vulkan/Shader.cpp
        src/vulkan/CommandExecutor.cpp
        src/vulkan/UI.cpp
        src/vulkan/MemoryAllocator.cpp
        src/vulkan/RenderPass.cpp
        src/vulkan/Instance.cpp
        src/vulkan/Verify.cpp
        src/vulkan/PhysicalDevice.cpp
        src/vulkan/Mesh.cpp
        src/vulkan/ExtensionSupport.cpp
        src/vulkan/Framebuffer.cpp
        src/vulkan/renderer/GraphicsRenderer.cpp
        src/vulkan/renderer/ResourceRenderer.cpp
        src/vulkan/renderer/Renderer.cpp
        src/vulkan/VertexBuffer.cpp
        src/vulkan/Swapchain.cpp
        src/vulkan/QueueFamilyIndex.cpp
        src/vulkan/UniformBuffer.cpp
        src/vulkan/Surface.cpp
        src/vulkan/Window.cpp
        src/vulkan/BaseBuffer.cpp
        src/vulkan/IndependentCommandExecutor.cpp
        src/vulkan/DebugConfigurator.cpp
        src/vulkan/Allocator.cpp
        src/vulkan/DebugMarker.cpp
        src/vulkan/Pipeline.cpp
        src/vulkan/Device.cpp
        src/vulkan/Material.cpp
        src/vulkan/ImageUtilities.cpp
        src/vulkan/Structures.cpp
        src/vulkan/IndexBuffer.cpp
        src/vulkan/Texture.cpp
        src/vulkan/Image.cpp
        src/imgui/InterfaceLayer.cpp)

if (DISARRAY_OS STREQUAL "Windows")
    list(APPEND ENGINE_SOURCES src/windows/UI.cpp)
elseif (DISARRAY_OS STREQUAL "MacOS")
    list(APPEND ENGINE_SOURCES src/linux/UI.cpp)
elseif (DISARRAY_OS STREQUAL "Linux")
    list(APPEND ENGINE_SOURCES src/linux/UI.cpp)
endif ()

add_library(
        ${PROJECT_NAME} STATIC
        ${THIRD_PARTY}/imgui/backends/imgui_impl_glfw.cpp
        ${THIRD_PARTY}/imgui/backends/imgui_impl_vulkan.cpp ${ENGINE_SOURCES})
add_library(Disarray::Engine ALIAS ${PROJECT_NAME})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE ${Vulkan_INCLUDE_DIRS}
        PUBLIC include . ${THIRD_PARTY}/glm ${THIRD_PARTY}/imgui ${THIRD_PARTY}/imguizmo)
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        glfw
        GPUOpen::VulkanMemoryAllocator
        imgui
        nlohmann_json::nlohmann_json
        tinyobjloader
        stb_image
        PUBLIC
        thread-pool
        Vulkan::Vulkan
        imguizmo
        magic_enum::magic_enum
        EnTT::EnTT
        fmt::fmt)
target_precompile_headers(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/DisarrayPCH.hpp)

default_compile_flags()

if (DISARRAY_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
