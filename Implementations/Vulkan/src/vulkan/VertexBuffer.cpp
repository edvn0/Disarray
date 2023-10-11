#include "DisarrayPCH.hpp"

#include "vulkan/VertexBuffer.hpp"

#include <vulkan/vulkan.h>

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"

namespace Disarray::Vulkan {

VertexBuffer::VertexBuffer(const Disarray::Device& dev, Disarray::BufferProperties properties)
	: Disarray::VertexBuffer(properties)
	, BaseBuffer(dev, BufferType::Vertex, properties)
{
}

} // namespace Disarray::Vulkan
