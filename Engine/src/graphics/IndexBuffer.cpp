#include "DisarrayPCH.hpp"

#include "graphics/IndexBuffer.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray {

	Ref<IndexBuffer> IndexBuffer::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::BufferProperties& props)
	{
		return make_ref<Vulkan::IndexBuffer>(device, swapchain, props);
	}

} // namespace Disarray
