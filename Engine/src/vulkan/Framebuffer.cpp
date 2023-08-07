#include "DisarrayPCH.hpp"

#include "vulkan/Framebuffer.hpp"

#include "Forward.hpp"
#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::Vulkan {

	Framebuffer::Framebuffer(Disarray::Device& dev, const FramebufferProperties& properties)
		: device(dev)
		, props(properties)
	{
		colour_count = props.colour_count;
		samples = props.samples;
		render_pass = props.optional_renderpass ? cast_to<Vulkan::RenderPass>(props.optional_renderpass)
												: make_ref<Vulkan::RenderPass>(device,
													RenderPassProperties {
														.image_format = props.format,
														.depth_format = props.depth_format,
														.samples = samples,
														.load_colour = props.load_colour,
														.keep_colour = props.keep_colour,
														.load_depth = props.load_depth,
														.keep_depth = props.keep_depth,
														.has_depth = props.has_depth,
														.should_present = props.should_present,
														.debug_name = "FromFramebuffer-" + props.debug_name,
													});

		create_attachments();
		recreate_framebuffer(false);
	}

	Framebuffer::~Framebuffer()
	{
		vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
		attachments.clear();
		if (props.has_depth) {
			depth_attachment.reset();
		}
	}

	void Framebuffer::force_recreation() { recreate_framebuffer(); }

	void Framebuffer::create_attachments()
	{
		auto total_colour_count = colour_count + static_cast<int>(samples != SampleCount::ONE);
		attachments.resize(total_colour_count);
		auto i = 0;
		for (auto& image : attachments) {
			image.reset(new Vulkan::Image(device,
				ImageProperties {
					.extent = props.extent,
					.format = props.format,
					.data = nullptr,
					.should_present = props.should_present,
					.samples = samples,
					.debug_name = props.debug_name + "-ColorFramebuffer-" + std::to_string(++i),
				}));
			clear_values.emplace_back().color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
		}
		if (props.has_depth) {
			depth_attachment.reset(new Vulkan::Image(device,
				ImageProperties { .extent = props.extent,
					.format = ImageFormat::Depth,
					.samples = samples,
					.debug_name = props.debug_name + "-DepthFramebuffer" }));
			clear_values.emplace_back().depthStencil = { 1.0f, 0 };
		}
	}

	void Framebuffer::recreate_framebuffer(bool should_clean)
	{
		if (should_clean) {
			vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
			for (auto& image : attachments) {
				image->force_recreation();
			}
			depth_attachment->force_recreation();
		}

		create_attachments();

		std::vector<VkImageView> fb_attachments;
		for (auto& img : attachments)
			fb_attachments.push_back(img->get_view());

		if (props.has_depth)
			fb_attachments.push_back(depth_attachment->get_view());

		auto framebuffer_info = vk_structures<VkFramebufferCreateInfo> {}();
		framebuffer_info.renderPass = render_pass->supply();
		framebuffer_info.attachmentCount = static_cast<std::uint32_t>(fb_attachments.size());
		framebuffer_info.pAttachments = fb_attachments.data();
		framebuffer_info.width = props.extent.width;
		framebuffer_info.height = props.extent.height;
		framebuffer_info.layers = 1;

		verify(vkCreateFramebuffer(supply_cast<Vulkan::Device>(device), &framebuffer_info, nullptr, &framebuffer));

		// Owned by someone else!
		if (!props.optional_renderpass)
			render_pass->force_recreation();
	}

} // namespace Disarray::Vulkan
