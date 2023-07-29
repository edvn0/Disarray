#include "graphics/IndexBuffer.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray {

	Ref<IndexBuffer> IndexBuffer::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,  const Disarray::IndexBufferProperties& props)
	{
		return make_ref<Vulkan::IndexBuffer>(device, swapchain, physical_device, props);
	}

}