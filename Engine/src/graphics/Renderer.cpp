#include "DisarrayPCH.hpp"

#include "graphics/Renderer.hpp"

#include "vulkan/Renderer.hpp"

namespace Disarray {

Ref<Renderer> Renderer::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const RendererProperties& props)
{
	return make_ref<Vulkan::Renderer>(device, swapchain, props);
}
} // namespace Disarray
