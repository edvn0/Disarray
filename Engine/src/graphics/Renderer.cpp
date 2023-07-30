#include "graphics/Renderer.hpp"

#include "vulkan/Renderer.hpp"

namespace Disarray {

	Ref<Renderer> Renderer::construct(
		Ref<Disarray::Device> device, Ref<Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, const RendererProperties& props)
	{
		return make_ref<Vulkan::Renderer>(device, swapchain, physical_device, props);
	}
} // namespace Disarray