#include "DisarrayPCH.hpp"

#include "graphics/VertexBuffer.hpp"

#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

	Ref<VertexBuffer> VertexBuffer::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::BufferProperties& props)
	{
		return make_ref<Vulkan::VertexBuffer>(device, swapchain, props);
	}

} // namespace Disarray
