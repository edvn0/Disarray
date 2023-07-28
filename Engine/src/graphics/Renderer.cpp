#include "graphics/Renderer.hpp"

#include "vulkan/Renderer.hpp"

namespace Disarray {

	Ref<Renderer> Renderer::construct(Ref<Disarray::Device> device, Ref<Swapchain> swapchain, const RendererProperties& props)
	{
		return make_ref<Vulkan::Renderer>(device, swapchain, props);
	}

}