#include "graphics/VertexBuffer.hpp"

#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

	Ref<VertexBuffer> VertexBuffer::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, const Disarray::VertexBufferProperties& props)
	{
		return make_ref<Vulkan::VertexBuffer>(device, swapchain, physical_device, props);
	}

}