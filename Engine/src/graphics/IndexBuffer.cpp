#include "graphics/IndexBuffer.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray {

	Ref<IndexBuffer> IndexBuffer::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::IndexBufferProperties& props)
	{
		return make_ref<Vulkan::IndexBuffer>(device, swapchain, props);
	}

}