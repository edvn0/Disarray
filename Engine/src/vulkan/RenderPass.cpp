#include "DisarrayPCH.hpp"

#include "vulkan/RenderPass.hpp"

#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

#include <array>

namespace Disarray::Vulkan {

	RenderPass::RenderPass(Disarray::Device& dev, const Disarray::RenderPassProperties& properties)
		: device(dev)
		, props(properties)
	{
		recreate_renderpass(false);
	}

	RenderPass::~RenderPass() { vkDestroyRenderPass(supply_cast<Vulkan::Device>(device), render_pass, nullptr); }

	void RenderPass::recreate_renderpass(bool should_clean)
	{
		if (should_clean)
			vkDestroyRenderPass(supply_cast<Vulkan::Device>(device), render_pass, nullptr);

		const auto has_msaa = props.samples != SampleCount::ONE;

		VkAttachmentDescription color_attachment {};
		color_attachment.format = to_vulkan_format(props.image_format);
		color_attachment.samples = to_vulkan_samples(props.samples);
		color_attachment.loadOp = props.load_colour ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = props.keep_colour ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_NONE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = props.load_colour ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = props.keep_colour ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

		// Disallow presenting with more than one sample!
		if (props.should_present && !has_msaa) {
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		VkAttachmentDescription depth_attachment {};
		depth_attachment.format = to_vulkan_format(props.depth_format);
		depth_attachment.samples = to_vulkan_samples(props.samples);
		depth_attachment.loadOp = props.load_depth ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = props.keep_depth ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = props.load_depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = props.keep_depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

		VkAttachmentReference color_attachment_ref {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref {};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// MSAA
		VkAttachmentDescription color_attachment_resolve {};
		color_attachment_resolve.format = to_vulkan_format(props.image_format);
		color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_resolve_ref {};
		color_attachment_resolve_ref.attachment = 2;
		color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// end

		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = props.has_depth ? &depth_attachment_ref : nullptr;
		subpass.pResolveAttachments = has_msaa ? &color_attachment_resolve_ref : nullptr;

		VkSubpassDependency dependency {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		if (props.has_depth) {
			dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		std::vector<VkAttachmentDescription> attachments = { color_attachment };
		if (props.has_depth)
			attachments.push_back(depth_attachment);
		if (has_msaa)
			attachments.push_back(color_attachment_resolve);

		auto render_pass_info = vk_structures<VkRenderPassCreateInfo> {}();
		render_pass_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		verify(vkCreateRenderPass(supply_cast<Vulkan::Device>(device), &render_pass_info, nullptr, &render_pass));
	}

} // namespace Disarray::Vulkan
