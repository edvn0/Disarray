#include "DisarrayPCH.hpp"

// clang-format off
#include "vulkan/Renderer.hpp"
// clang-format on

#include "core/Clock.hpp"
#include "core/Types.hpp"
#include "graphics/PipelineCache.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/VertexBuffer.hpp"

#include <array>
#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const GeometryProperties& properties)
	{
		draw_mesh(executor, mesh, properties.to_transform());
	}

	void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const glm::mat4& transform)
	{
		draw_mesh(executor, mesh, mesh.get_pipeline(), transform);
	}

	void Renderer::draw_mesh(
		Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline, const glm::mat4& transform)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
		const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

		pc.object_transform = transform;
		vkCmdPushConstants(
			command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

		const std::array<VkDescriptorSet, 1> desc { get_descriptor_set() };
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_layout(), 0, 1, desc.data(), 0, nullptr);

		std::array<VkBuffer, 1> arr;
		arr[0] = supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices());
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(mesh.get_indices()), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(mesh.get_indices().size()), 1, 0, 0, 0);
	}

	void Renderer::end_pass(Disarray::CommandExecutor& executor)
	{
		submit_batched_geometry(executor);
		vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor));
	}

	void Renderer::begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& fb, bool explicit_clear)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

		VkRenderPassBeginInfo render_pass_begin_info {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = supply_cast<Vulkan::RenderPass>(fb.get_render_pass());
		render_pass_begin_info.framebuffer = supply_cast<Vulkan::Framebuffer>(fb);
		render_pass_begin_info.renderArea.offset = { 0, 0 };

		VkExtent2D extent_2_d { .width = extent.width, .height = extent.height };
		render_pass_begin_info.renderArea.extent = extent_2_d;

		auto clear_values = cast_to<Vulkan::Framebuffer>(fb).get_clear_values();

		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		if (explicit_clear) {
			auto& vk_framebuffer = cast_to<Vulkan::Framebuffer>(fb);

			std::vector<VkClearValue> fb_clear_values = vk_framebuffer.get_clear_values();

			const std::uint32_t color_attachment_count = vk_framebuffer.get_colour_attachment_count();
			const std::uint32_t total_attachment_count = color_attachment_count + (vk_framebuffer.has_depth() ? 1 : 0);

			std::vector<VkClearAttachment> attachments(total_attachment_count);
			std::vector<VkClearRect> clear_rects(total_attachment_count);
			for (uint32_t i = 0; i < color_attachment_count; i++) {
				attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				attachments[i].colorAttachment = i;
				attachments[i].clearValue = fb_clear_values[i];

				clear_rects[i].rect.offset = { (int32_t)0, (int32_t)0 };
				clear_rects[i].rect.extent = extent_2_d;
				clear_rects[i].baseArrayLayer = 0;
				clear_rects[i].layerCount = 1;
			}

			if (vk_framebuffer.has_depth()) {
				attachments[color_attachment_count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				attachments[color_attachment_count].clearValue = fb_clear_values[color_attachment_count];
				clear_rects[color_attachment_count].rect.offset = { 0, 0 };
				clear_rects[color_attachment_count].rect.extent = extent_2_d;
				clear_rects[color_attachment_count].baseArrayLayer = 0;
				clear_rects[color_attachment_count].layerCount = 1;
			}

			vkCmdClearAttachments(command_buffer, total_attachment_count, attachments.data(), total_attachment_count, clear_rects.data());
		}

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

} // namespace Disarray::Vulkan
