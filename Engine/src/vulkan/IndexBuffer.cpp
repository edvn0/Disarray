#include "DisarrayPCH.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray::Vulkan {

	IndexBuffer::IndexBuffer(Disarray::Device& dev, Disarray::Swapchain& swapchain, const BufferProperties& properties)
		: BaseBuffer(dev, swapchain, BufferType::Index, properties)
	{
	}

} // namespace Disarray::Vulkan
