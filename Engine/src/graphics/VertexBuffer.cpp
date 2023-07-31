#include "graphics/VertexBuffer.hpp"

#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

	Ref<VertexBuffer> VertexBuffer::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::VertexBufferProperties& props)
	{
		return make_ref<Vulkan::VertexBuffer>(device, swapchain, props);
	}

}