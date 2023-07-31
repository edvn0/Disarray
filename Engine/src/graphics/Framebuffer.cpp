#include "graphics/Framebuffer.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "graphics/RenderPass.hpp"
#include "vulkan/Framebuffer.hpp"

namespace Disarray {

	Ref<Framebuffer> Framebuffer::construct(Disarray::Device& device,Disarray::Swapchain& swapchain, const Disarray::FramebufferProperties& props)
	{
		return make_ref<Vulkan::Framebuffer>(device, swapchain, props);
	}

}