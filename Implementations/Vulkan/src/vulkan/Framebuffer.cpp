#include "DisarrayPCH.hpp"

#include "vulkan/Framebuffer.hpp"

#include <vulkan/Allocator.hpp>

#include <core/Ensure.hpp>

#include <memory>

#include "Forward.hpp"
#include "core/Types.hpp"
#include "fmt/core.h"
#include "graphics/ImageProperties.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/DebugMarker.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::Vulkan {

Framebuffer::Framebuffer(const Disarray::Device& dev, FramebufferProperties properties)
	: Disarray::Framebuffer(std::move(properties))
	, device(dev)
{
	std::uint32_t attachment_index = 0;
	for (auto& attachment_spec : props.attachments.texture_attachments) {
		if (is_depth_format(attachment_spec.format)) {
			TextureProperties spec {};
			spec.format = attachment_spec.format;
			spec.extent = props.extent;
			spec.debug_name = fmt::format("{0}-depth{1}", props.debug_name, attachment_index);
			spec.sampler_modes = {
				SamplerMode::ClampToBorder,
				SamplerMode::ClampToBorder,
				SamplerMode::ClampToBorder,
			};
			spec.border_colour = BorderColour::FloatOpaqueWhite;
			depth_attachment = make_scope<Vulkan::Texture>(device, std::move(spec));
		} else {
			TextureProperties spec {};
			spec.format = attachment_spec.format;
			spec.extent = props.extent;
			spec.debug_name = fmt::format("{0}-color{1}", props.debug_name, attachment_index);
			attachments.emplace_back(make_scope<Vulkan::Texture>(device, std::move(spec)));
		}
		attachment_index++;
	}

	if (!props.scissors.is_valid()) {
		props.scissors = {
			.extent = props.extent,
			.offset = { 0, 0 },
		};
	}

	colour_count = static_cast<std::uint32_t>(attachments.size());
	recreate_framebuffer(false);
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
	attachments.clear();
	if (depth_attachment) {
		depth_attachment.reset();
	}
}

void Framebuffer::force_recreation() { recreate_framebuffer(); }

void Framebuffer::recreate_framebuffer(bool should_clean)
{
	if (should_clean) {
		vkDestroyFramebuffer(supply_cast<Vulkan::Device>(device), framebuffer, nullptr);
		for (auto& image : attachments) {
			image->force_recreation();
		}
		if (depth_attachment) {
			depth_attachment->force_recreation();
		}
	}

	std::vector<VkAttachmentDescription> attachment_descriptions;
	std::vector<VkAttachmentReference> color_attachment_references;
	VkAttachmentReference depth_attachment_reference {};

	clear_values.resize(props.attachments.texture_attachments.size());
	uint32_t attachment_index = 0;
	for (const auto& attachment_spec : props.attachments.texture_attachments) {
		const auto is_depth = is_depth_format(attachment_spec.format);
		if (is_depth) {
			depth_attachment->recreate(true, props.extent);
			VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
			attachment_description.flags = 0;
			attachment_description.format = to_vulkan_format(attachment_spec.format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment_description.loadOp = props.clear_depth_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment_description.initialLayout
				= props.clear_depth_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			depth_attachment_reference = { .attachment = attachment_index, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			clear_values[attachment_index].depthStencil = { props.depth_clear_value, 0 };

			const auto depth_is_last_index = props.attachments.texture_attachments.size() - 1;
			ensure(attachment_index == depth_is_last_index, "Depth image must always be last...");
		} else {
			auto& image = attachments[attachment_index];
			image->recreate(true, props.extent);

			VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
			attachment_description.flags = 0;
			attachment_description.format = to_vulkan_format(attachment_spec.format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment_description.loadOp = props.clear_colour_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment_description.initialLayout = props.clear_colour_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			const auto& clear_color = props.clear_colour;
			clear_values[attachment_index].color = { { clear_color.r, clear_color.g, clear_color.b, clear_color.a } };
			color_attachment_references.emplace_back(VkAttachmentReference { attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			attachment_index++;
		}
	}

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = uint32_t(color_attachment_references.size());
	subpass_description.pColorAttachments = color_attachment_references.data();
	if (depth_attachment) {
		subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
	}

	std::vector<VkSubpassDependency> dependencies;
	if (!attachments.empty()) {
		{
			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
		{
			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
	}

	if (depth_attachment) {
		{
			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		{
			VkSubpassDependency& dependency = dependencies.emplace_back();
			dependency.srcSubpass = 0;
			dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
	}

	auto render_pass_info = vk_structures<VkRenderPassCreateInfo>()();
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
	render_pass_info.pAttachments = attachment_descriptions.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;
	// render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
	// render_pass_info.pDependencies = dependencies.data();

	render_pass = RenderPass::construct(device,
		{
			.debug_name = fmt::format("RenderPass-{}", props.debug_name),
		});
	auto& vk_render_pass = cast_to<Vulkan::RenderPass>(*render_pass);
	vk_render_pass.create_with(render_pass_info);

	std::vector<VkImageView> attachment_views;
	for (auto& image : attachments) {
		auto& view = attachment_views.emplace_back();
		view = cast_to<Vulkan::Image>(image->get_image(0)).get_descriptor_info().imageView;
	}

	if (depth_attachment) {
		auto& depth_view = attachment_views.emplace_back();
		depth_view = cast_to<Vulkan::Image>(depth_attachment->get_image(0)).get_descriptor_info().imageView;
	}

	auto framebuffer_create_info = vk_structures<VkFramebufferCreateInfo>()();
	framebuffer_create_info.renderPass = vk_render_pass.supply();
	framebuffer_create_info.attachmentCount = static_cast<std::uint32_t>(attachment_views.size());
	framebuffer_create_info.pAttachments = attachment_views.data();
	framebuffer_create_info.width = props.extent.width;
	framebuffer_create_info.height = props.extent.height;
	framebuffer_create_info.layers = 1;

	verify(vkCreateFramebuffer(supply_cast<Vulkan::Device>(device), &framebuffer_create_info, nullptr, &framebuffer));
	DebugMarker::set_object_name(
		supply_cast<Vulkan::Device>(device), framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, props.debug_name.c_str());
}

} // namespace Disarray::Vulkan
