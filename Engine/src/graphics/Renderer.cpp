#include "DisarrayPCH.hpp"

#include "graphics/Renderer.hpp"

#include "graphics/Framebuffer.hpp"
#include "graphics/ImageProperties.hpp"
#include "vulkan/Renderer.hpp"

namespace Disarray {

RenderAreaExtent::RenderAreaExtent(const Disarray::Framebuffer& framebuffer)
	: offset({ 0, 0 })
	, extent(framebuffer.get_properties().extent)
{
}

RenderAreaExtent::RenderAreaExtent(const Disarray::Extent& offset, const Disarray::Extent& extent)
	: offset(offset)
	, extent(extent)
{
}

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
