#include "vulkan/Renderer.hpp"

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/VertexBuffer.hpp"

#include <array>

namespace Disarray::Vulkan {

	Renderer::Renderer(
		Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> sc, Ref<Disarray::PhysicalDevice> pd, const Disarray::RendererProperties& properties)
		: device(dev)
		, swapchain(sc)
		, props(properties)
		, pipeline_cache(device, "Assets/Shaders")
	{
	}

	Renderer::~Renderer() { Log::debug("Renderer destroyed!"); }

	void Renderer::set_extent(const Extent& e) { extent = e; }

	void Renderer::begin_pass(Ref<Disarray::CommandExecutor> command_executor, Ref<Disarray::RenderPass> render_pass, Ref<Disarray::Framebuffer> fb)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

		VkRenderPassBeginInfo render_pass_begin_info {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = supply_cast<Vulkan::RenderPass>(render_pass);
		render_pass_begin_info.framebuffer = supply_cast<Vulkan::Framebuffer>(fb);
		render_pass_begin_info.renderArea.offset = { 0, 0 };

		VkExtent2D extent_2_d { .width = extent.width, .height = extent.height };
		render_pass_begin_info.renderArea.extent = extent_2_d;

		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clear_values[1].depthStencil = { 1.0f, 0 };

		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent = extent_2_d;
		vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	}

	void Renderer::end_pass(Ref<Disarray::CommandExecutor> executor) { vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor)); }

	void Renderer::draw_mesh(Ref<Disarray::CommandExecutor> executor, Ref<Disarray::Mesh> mesh)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
		const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh->get_pipeline());
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->supply());

		vkCmdPushConstants(
			command_buffer, pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

		std::array<VkBuffer, 1> arr;
		arr[0] = supply_cast<Vulkan::VertexBuffer>(mesh->get_vertices());
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(mesh->get_indices()), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, mesh->get_indices()->size(), 1, 0, 0, 0);
	}

	void Renderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties) { }

} // namespace Disarray::Vulkan