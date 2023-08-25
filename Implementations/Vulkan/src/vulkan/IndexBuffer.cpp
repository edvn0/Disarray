#include "DisarrayPCH.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray::Vulkan {

IndexBuffer::IndexBuffer(const Disarray::Device& dev, const BufferProperties& properties)
	: BaseBuffer(dev, BufferType::Index, properties)
{
}

} // namespace Disarray::Vulkan
