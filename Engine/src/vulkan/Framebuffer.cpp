#include "vulkan/Framebuffer.hpp"

#include "core/Types.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Swapchain.hpp"

namespace Disarray::Vulkan {

	Framebuffer::Framebuffer(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::RenderPass> rp,
		const Disarray::FramebufferProperties& properties)
		: device(dev)
		, swapchain(cast_to<Vulkan::Swapchain>(swapchain))
		, render_pass(rp)
		, props(properties)
	{
		recreate(false);
	}

	Framebuffer::~Framebuffer()
	{
		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
		}
	}

	void Framebuffer::force_recreation() { recreate(); }

	void Framebuffer::recreate(bool should_clean)
	{
		if (should_clean) {
			for (auto framebuffer : framebuffers) {
				vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
			}
		}
		framebuffers.resize(swapchain->image_count());

		const auto& views = cast_to<Vulkan::Swapchain>(swapchain)->get_views();
		for (size_t i = 0; i < views.size(); i++) {
			VkImageView attachments[] = { views[i] };

			VkFramebufferCreateInfo framebuffer_info {};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = supply_cast<Vulkan::RenderPass>(render_pass);
			framebuffer_info.attachmentCount = 1;
			framebuffer_info.pAttachments = attachments;
			framebuffer_info.width = swapchain->get_extent().width;
			framebuffer_info.height = swapchain->get_extent().height;
			framebuffer_info.layers = 1;

			verify(vkCreateFramebuffer(supply_cast<Vulkan::Device>(device), &framebuffer_info, nullptr, &framebuffers[i]));
		}
	}
} // namespace Disarray::Vulkan