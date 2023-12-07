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

Renderer::Renderer(Scope<IGraphicsResource> resource)
	: graphics_resource { std::move(resource) }
{
	DataBuffer data_buffer;
	data_buffer.allocate(sizeof(std::uint32_t));
	data_buffer.write(0xFFFFFFFF);

	white_texture = Texture::construct(graphics_resource->get_device(),
		{
			.extent = { 1, 1 },
			.data_buffer = data_buffer,
			.debug_name = "White Texture",
		});

	data_buffer.allocate(sizeof(std::uint32_t));
	data_buffer.write(0x00000000);

	black_texture = Texture::construct(graphics_resource->get_device(),
		{
			.extent = { 1, 1 },
			.data_buffer = data_buffer,
			.debug_name = "Black Texture",
		});
}

Renderer::~Renderer()
{
	white_texture.reset();
	black_texture.reset();
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
