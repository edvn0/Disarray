#include "vulkan/Framebuffer.hpp"

#include "Forward.hpp"
#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::Vulkan {

	Framebuffer::Framebuffer(Disarray::Device& dev, Disarray::Swapchain& sc, const FramebufferProperties& properties)
		: swapchain(sc)
		, device(dev)
		, props(properties)
	{
		render_pass = RenderPass::construct(device,
			{
				.image_format = props.format,
				.depth_format = props.depth_format,
				.load_colour = props.load_colour,
				.keep_colour = props.keep_colour,
				.load_depth = props.load_depth,
				.keep_depth = props.keep_depth,
				.has_depth = props.has_depth,
				.should_present = props.should_present,
				.debug_name = "FromFramebuffer-" + props.debug_name,
			});
		if (props.has_depth) {
			depth_texture = Disarray::Texture::construct(device, swapchain,
				{ .extent = swapchain.get_extent(), .format = ImageFormat::Depth, .debug_name = props.debug_name + "-DepthFramebuffer" });
		}
		create_attachment_textures();
		recreate_framebuffer(false);
	}

	Framebuffer::~Framebuffer()
	{
		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
		}
		textures.clear();
	}

	void Framebuffer::force_recreation() { recreate_framebuffer(); }

	void Framebuffer::create_attachment_textures()
	{
		textures.resize(swapchain.image_count());
		auto i = 0;
		for (auto& texture : textures) {
			texture = Disarray::Texture::construct(device, swapchain,
				TextureProperties {
					.extent = swapchain.get_extent(),
					.format = props.format,
					.should_present = props.should_present,
					.debug_name = props.debug_name + "-ColorFramebuffer-" + std::to_string(++i),
				});
		}
	}

	void Framebuffer::recreate_framebuffer(bool should_clean)
	{
		if (should_clean) {
			for (auto framebuffer : framebuffers) {
				vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
			}
		}

		for (auto& tex : textures) {
			tex->recreate(should_clean);
		}
		if (depth_texture)
			depth_texture->recreate(should_clean);

		framebuffers.resize(swapchain.image_count());
		for (size_t i = 0; i < framebuffers.size(); i++) {
			auto texture = textures[i].as<Vulkan::Texture>();
			std::vector<VkImageView> attachments = { texture->get_view() };
			if (props.has_depth)
				attachments.push_back(depth_texture.as<Vulkan::Texture>()->get_view());

			VkFramebufferCreateInfo framebuffer_info {};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = *render_pass.as<Vulkan::RenderPass>();
			framebuffer_info.attachmentCount = attachments.size();
			framebuffer_info.pAttachments = attachments.data();
			framebuffer_info.width = swapchain.get_extent().width;
			framebuffer_info.height = swapchain.get_extent().height;
			framebuffer_info.layers = 1;

			verify(vkCreateFramebuffer(supply_cast<Vulkan::Device>(device), &framebuffer_info, nullptr, &framebuffers[i]));
		}
	}

} // namespace Disarray::Vulkan
