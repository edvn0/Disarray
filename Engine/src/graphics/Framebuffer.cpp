#include "graphics/Framebuffer.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "graphics/RenderPass.hpp"
#include "vulkan/Framebuffer.hpp"

namespace Disarray {

	Ref<Framebuffer> Framebuffer::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, Ref<RenderPass> render_pass, const Disarray::FramebufferProperties& props)
	{
		return make_ref<Vulkan::Framebuffer>(device, swapchain,physical_device,render_pass, props);
	}

}