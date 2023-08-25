#include "DisarrayPCH.hpp"

#include "vulkan/UniformBuffer.hpp"

#include <vulkan/vulkan.h>

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"

namespace Disarray::Vulkan {

UniformBuffer::UniformBuffer(const Disarray::Device& dev, const Disarray::BufferProperties& properties)
	: BaseBuffer(dev, BufferType::Uniform, properties)
{
	create_buffer_info();
}

void UniformBuffer::create_buffer_info()
{
	buffer_info.buffer = supply();
	buffer_info.offset = 0;
	buffer_info.range = size();
}

} // namespace Disarray::Vulkan
