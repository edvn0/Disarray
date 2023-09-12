#include "DisarrayPCH.hpp"

#include "graphics/Renderer.hpp"

#include "vulkan/Renderer.hpp"

namespace Disarray {

auto Renderer::construct(const Disarray::Device& device, const Disarray::Swapchain& swapchain, const RendererProperties& props)
	-> Ref<Disarray::Renderer>
{
	return make_ref<Vulkan::Renderer>(device, swapchain, props);
}

auto Renderer::construct_unique(const Disarray::Device& device, const Disarray::Swapchain& swapchain, const RendererProperties& props)
	-> Scope<Disarray::Renderer>
{
	return make_scope<Vulkan::Renderer>(device, swapchain, props);
}

} // namespace Disarray
