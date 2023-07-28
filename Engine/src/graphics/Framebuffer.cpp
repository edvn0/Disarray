#include "graphics/Framebuffer.hpp"

#include "graphics/RenderPass.hpp"
#include "vulkan/Framebuffer.hpp"

namespace Disarray {

	Ref<Framebuffer> Framebuffer::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<RenderPass> render_pass, const Disarray::FramebufferProperties& props)
	{
		return make_ref<Vulkan::Framebuffer>(device, swapchain,render_pass, props);
	}

}