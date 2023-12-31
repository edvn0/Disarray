#include "DisarrayPCH.hpp"

#include "vulkan/RenderPass.hpp"

#include <vulkan/DebugMarker.hpp>
#include <vulkan/vulkan.h>

#include <core/Ensure.hpp>

#include "core/Types.hpp"
#include "graphics/RenderPass.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Verify.hpp"

namespace Disarray::Vulkan {

RenderPass::RenderPass(const Disarray::Device& dev, Disarray::RenderPassProperties properties)
	: Disarray::RenderPass(std::move(properties))
	, device(dev)
{
}

void RenderPass::create_with(VkRenderPassCreateInfo render_pass_create_info)
{
	render_pass_info = render_pass_create_info;
	verify(vkCreateRenderPass(supply_cast<Vulkan::Device>(device), &render_pass_info, nullptr, &render_pass));
	DebugMarker::set_object_name(
		supply_cast<Vulkan::Device>(device), render_pass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, props.debug_name.c_str());
}

RenderPass::~RenderPass() { vkDestroyRenderPass(supply_cast<Vulkan::Device>(device), render_pass, nullptr); }

void RenderPass::recreate_renderpass(bool should_clean)
{
	ensure(render_pass != nullptr);

	if (should_clean)
		vkDestroyRenderPass(supply_cast<Vulkan::Device>(device), render_pass, nullptr);

	verify(vkCreateRenderPass(supply_cast<Vulkan::Device>(device), &render_pass_info, nullptr, &render_pass));
}

} // namespace Disarray::Vulkan
