#include "DisarrayPCH.hpp"

#include "vulkan/VertexBuffer.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	VertexBuffer::VertexBuffer(Disarray::Device& dev, const Disarray::BufferProperties& properties)
		: BaseBuffer(dev, BufferType::Vertex, properties)
	{
	}

} // namespace Disarray::Vulkan
