#include "DisarrayPCH.hpp"

#include "vulkan/StorageBuffer.hpp"

namespace Disarray::Vulkan {

StorageBuffer::StorageBuffer(const Disarray::Device& dev, BufferProperties properties)
	: Disarray::StorageBuffer(properties)
	, BaseBuffer(dev, BufferType::Storage, properties)
{
	info.buffer = supply();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;
}

} // namespace Disarray::Vulkan
