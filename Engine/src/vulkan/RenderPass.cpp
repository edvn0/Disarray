#include "vulkan/RenderPass.hpp"

#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Verify.hpp"

namespace Disarray::Vulkan {

	static constexpr auto to_vulkan_format(ImageFormat format)
	{
		switch (format) {
		case ImageFormat::SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::RGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case ImageFormat::SBGR:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case ImageFormat::BGR:
			return VK_FORMAT_B8G8R8_SRGB;
		case ImageFormat::Depth:
			return VK_FORMAT_D16_UNORM;
		case ImageFormat::DepthStencil:
			unreachable();
		default:
			unreachable();
		}
	}

	RenderPass::RenderPass(Ref<Disarray::Device> dev, const Disarray::RenderPassProperties& properties)
		: device(dev)
		, props(properties)
	{
		  recreate(false);
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(supply_cast<Vulkan::Device>(device), render_pass, nullptr);
	}

	void RenderPass::recreate(bool should_clean) {
		if (should_clean) vkDestroyRenderPass(supply_cast<Vulkan::Device>(device), render_pass, nullptr);

		VkAttachmentDescription color_attachment {};
		color_attachment.format = to_vulkan_format(props.image_format);
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkRenderPassCreateInfo render_pass_info {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		verify(vkCreateRenderPass(supply_cast<Vulkan::Device>(device), &render_pass_info, nullptr, &render_pass));
	}

} // namespace Disarray::Vulkan