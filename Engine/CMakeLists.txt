cmake_minimum_required(VERSION 3.22)

project(Engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_SOURCES
        Disarray.hpp
        include/core/AllocatorConfigurator.hpp
        include/core/App.hpp
        include/core/CleanupAwaiter.hpp
        include/core/Clock.hpp
        include/core/Collections.hpp
        include/core/Concepts.hpp
        include/core/DataBuffer.hpp
        include/core/DebugConfigurator.hpp
        include/core/DisarrayObject.hpp
        include/core/Ensure.hpp
        include/core/Entry.hpp
        include/core/events/ApplicationEvent.hpp
        include/core/events/Event.hpp
        include/core/events/KeyEvent.hpp
        include/core/events/MouseEvent.hpp
        include/core/exceptions/BaseException.hpp
        include/core/exceptions/GeneralExceptions.hpp
        include/core/filesystem/AssetLocations.hpp
        include/core/filesystem/FileIO.hpp
        include/core/FileWatcher.hpp
        include/core/Formatters.hpp
        include/core/Hashes.hpp
        include/core/Input.hpp
        include/core/Instrumentation.hpp
        include/core/KeyCode.hpp
        include/core/Layer.hpp
        include/core/Log.hpp
        include/core/MouseCode.hpp
        include/core/Panel.hpp
        include/core/Platform.hpp
        include/core/PointerDefinition.hpp
        include/core/PolymorphicCast.hpp
        include/core/Random.hpp
        include/core/ReferenceCounted.hpp
        include/core/ThreadPool.hpp
        include/core/Tuple.hpp
        include/core/Types.hpp
        include/core/String.hpp
        include/core/UniquelyIdentifiable.hpp
        include/core/UsageBadge.hpp
        include/core/Window.hpp
        include/DisarrayPCH.hpp
        include/Forward.hpp
        include/graphics/AABB.hpp
        include/graphics/BufferProperties.hpp
        include/graphics/BufferSet.hpp
        include/graphics/CommandExecutor.hpp
        include/graphics/Device.hpp
        include/graphics/Extent.hpp
        include/graphics/Framebuffer.hpp
        include/graphics/GLM.hpp
        include/graphics/Image.hpp
        include/graphics/ImageLoader.hpp
        include/graphics/ImageProperties.hpp
        include/graphics/IndexBuffer.hpp
        include/graphics/Instance.hpp
        include/graphics/Material.hpp
        include/graphics/Maths.hpp
        include/graphics/Mesh.hpp
        include/graphics/MeshMaterial.hpp
        include/graphics/ModelLoader.hpp
        include/graphics/model_loaders/AssimpModelLoader.hpp
        include/graphics/model_loaders/TinyObjModelLoader.hpp
        include/graphics/ModelVertex.hpp
        include/graphics/PhysicalDevice.hpp
        include/graphics/PipelineCache.hpp
        include/graphics/Pipeline.hpp
        include/graphics/PushConstantLayout.hpp
        include/graphics/QueueFamilyIndex.hpp
        include/graphics/RenderBatch.hpp
        include/graphics/RenderCommandQueue.hpp
        include/graphics/Renderer.hpp
        include/graphics/RendererProperties.hpp
        include/graphics/RenderPass.hpp
        include/graphics/ResourceCache.hpp
        include/graphics/ShaderCompiler.hpp
        include/graphics/Shader.hpp
        include/graphics/SingleShader.hpp
        include/graphics/StaticMesh.hpp
        include/graphics/StorageBuffer.hpp
        include/graphics/StaticMesh.hpp
        include/graphics/Surface.hpp
        include/graphics/Swapchain.hpp
        include/graphics/TextRenderer.hpp
        include/graphics/TextureCache.hpp
        include/graphics/Texture.hpp
        include/graphics/UniformBuffer.hpp
        include/graphics/UniformBufferSet.hpp
        include/graphics/VertexBuffer.hpp
        include/graphics/VertexTypes.hpp
        include/physics/PhysicsEngine.hpp
        include/physics/PhysicsProperties.hpp
        include/scene/Camera.hpp
        include/scene/Component.hpp
        include/scene/ComponentSerialisers.hpp
        include/scene/Components.hpp
        include/scene/CppScript.hpp
        include/scene/Deserialiser.hpp
        include/scene/Entity.hpp
        include/scene/Scene.hpp
        include/scene/SceneRenderer.hpp
        include/scene/Scripts.hpp
        include/scene/SerialisationTypeConversions.hpp
        include/scene/Serialiser.hpp
        include/ui/InterfaceLayer.hpp
        include/ui/UI.hpp
        include/util/BitCast.hpp
        include/util/FormattingUtilities.hpp
        include/util/Timer.hpp
        src/core/App.cpp
        src/core/DataBuffer.cpp
        src/core/exceptions/BaseException.cpp
        src/core/filesystem/AssetLocations.cpp
        src/core/filesystem/FileIO.cpp
        src/core/FileWatcher.cpp
        src/core/Formatters.cpp
        src/core/Instrumentation.cpp
        src/core/Layer.cpp
        src/core/Log.cpp
        src/core/ReferenceCounted.cpp
        src/core/Types.cpp
        src/core/String.cpp
        src/core/Window.cpp
        src/glfw/GLFWClock.cpp
        src/glfw/GLFWInput.cpp
        src/graphics/AABB.cpp
        src/graphics/BufferSet.cpp
        src/graphics/CommandExecutor.cpp
        src/graphics/Device.cpp
        src/graphics/Framebuffer.cpp
        src/graphics/Image.cpp
        src/graphics/ImageLoader.cpp
        src/graphics/ImageProperties.cpp
        src/graphics/IndexBuffer.cpp
        src/graphics/Instance.cpp
        src/graphics/Material.cpp
        src/graphics/Maths.cpp
        src/graphics/Mesh.cpp
        src/graphics/MeshMaterial.cpp
        src/graphics/ModelLoader.cpp
        src/graphics/model_loaders/AssimpModelLoader.cpp
        src/graphics/model_loaders/TinyObjModelLoader.cpp
        src/graphics/PhysicalDevice.cpp
        src/graphics/PipelineCache.cpp
        src/graphics/Pipeline.cpp
        src/graphics/PushConstantLayout.cpp
        src/graphics/QueueFamilyIndex.cpp
        src/graphics/RenderCommandQueue.cpp
        src/graphics/Renderer.cpp
        src/graphics/RendererProperties.cpp
        src/graphics/RenderPass.cpp
        src/graphics/Shader.cpp
        src/graphics/SingleShader.cpp
        src/graphics/StorageBuffer.cpp
        src/graphics/StaticMesh.cpp
        src/graphics/Swapchain.cpp
        src/graphics/Texture.cpp
        src/graphics/UniformBuffer.cpp
        src/graphics/UniformBufferSet.cpp
        src/graphics/VertexBuffer.cpp
        src/imgui/InterfaceLayer.cpp
        src/physics/PhysicsEngine.cpp
        src/scene/Camera.cpp
        src/scene/ComponentDeserialisers.cpp
        src/scene/Components.cpp
        src/scene/ComponentSerialisers.cpp
        src/scene/CppScript.cpp
        src/scene/Deserialiser.cpp
        src/scene/Entity.cpp
        src/scene/Scene.cpp
        src/scene/SceneRenderer.cpp
        src/scene/Scripts.cpp
        src/scene/SerialisationTypeConversions.cpp
        src/util/FormattingUtilities.cpp
        src/windows/UI.cpp
)

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
        PUBLIC include . ${THIRD_PARTY}/glm ${THIRD_PARTY}/spdlog/include ${THIRD_PARTY}/stb_image/include)
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE glfw GPUOpen::VulkanMemoryAllocator nlohmann_json::nlohmann_json
        tinyobjloader
        stb_image
        spdlog::spdlog
        Implementations::Vulkan
        thread-pool
        assimp::assimp
        Vulkan::Vulkan
        PUBLIC
        fmt::fmt
        imgui
        libtinyfiledialogs
        imguizmo
        magic_enum::magic_enum
        EnTT::EnTT
        freetype
        reactphysics3d
)
target_precompile_headers(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/DisarrayPCH.hpp)

default_compile_flags()

if (DISARRAY_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
