#include "vulkan/Renderer.hpp"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	Renderer::Renderer(Ref<Disarray::Device> dev,Ref<Disarray::Swapchain> sc, const Disarray::RendererProperties& properties)
		: device(dev)
		,swapchain(sc)
		, props(properties)
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::set_extent(const Extent& e) {
		extent = e;
	}

	void Renderer::begin_pass(Ref<Disarray::CommandExecutor> command_executor, Ref<Disarray::RenderPass> render_pass, Ref<Disarray::Pipeline> pipeline, Ref<Disarray::Framebuffer> fb)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

		VkRenderPassBeginInfo render_pass_begin_info {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = supply_cast<Vulkan::RenderPass>(render_pass);
		render_pass_begin_info.framebuffer = supply_cast<Vulkan::Framebuffer>(fb);
		render_pass_begin_info.renderArea.offset = {0, 0};

		VkExtent2D extent_2_d {.width = extent.width, .height =extent.height};
		render_pass_begin_info.renderArea.extent = extent_2_d;

		VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, supply_cast<Vulkan::Pipeline>(pipeline));
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = extent_2_d;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}

	void Renderer::end_pass(Ref<Disarray::CommandExecutor> executor) {
		vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor));
	}

	void Renderer::draw_planar_geometry(Ref<Disarray::CommandExecutor> executor, Geometry geometry, const GeometryProperties& properties) {
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
		if (geometry == Geometry::Triangle) {
			vkCmdDraw(command_buffer, 3, 1, 0, 0);
		}
	}

} // namespace Disarray::Vulkan