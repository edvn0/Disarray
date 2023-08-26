#include "DisarrayPCH.hpp"

#include "graphics/Renderer.hpp"

#include "vulkan/Renderer.hpp"

namespace Disarray {

Ref<Renderer> Renderer::construct(const Disarray::Device& device, const Disarray::Swapchain& swapchain, const RendererProperties& props)
{
	return make_ref<Vulkan::Renderer>(device, swapchain, props);
}

Scope<Renderer> Renderer::construct_unique(const Disarray::Device& device, const Disarray::Swapchain& swapchain, const RendererProperties& props)
{
	return make_scope<Vulkan::Renderer>(device, swapchain, props);
}

} // namespace Disarray
