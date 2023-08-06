#include "DisarrayPCH.hpp"

#include "vulkan/VertexBuffer.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	VertexBuffer::VertexBuffer(Disarray::Device& dev, Disarray::Swapchain& swapchain, const Disarray::BufferProperties& properties)
		: BaseBuffer(dev, swapchain, BufferType::Vertex, properties)
	{
	}

} // namespace Disarray::Vulkan
