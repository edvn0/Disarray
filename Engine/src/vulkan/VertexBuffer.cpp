#include "DisarrayPCH.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/VertexBuffer.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

VertexBuffer::VertexBuffer(const Disarray::Device& dev, const Disarray::BufferProperties& properties)
	: BaseBuffer(dev, BufferType::Vertex, properties)
{
}

} // namespace Disarray::Vulkan
