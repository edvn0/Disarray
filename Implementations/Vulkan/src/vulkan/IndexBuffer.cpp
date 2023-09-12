#include "DisarrayPCH.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray::Vulkan {

IndexBuffer::IndexBuffer(const Disarray::Device& dev, BufferProperties properties)
	: Disarray::IndexBuffer(properties)
	, BaseBuffer(dev, BufferType::Index, properties)
{
}

} // namespace Disarray::Vulkan
