#include "DisarrayPCH.hpp"

// clang-format off
#include "graphics/IndexBuffer.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/Renderer.hpp"
// clang-format on

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>

#include "core/Types.hpp"
#include "graphics/Pipeline.hpp"
#include "util/BitCast.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray::Vulkan {

void Renderer::bind_pipeline(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, Disarray::PipelineBindPoint point)
{
	if (&pipeline != bound_pipeline) {
		bound_pipeline = &pipeline;
		vkCmdBindPipeline(
			supply_cast<Vulkan::CommandExecutor>(executor), static_cast<VkPipelineBindPoint>(point), supply_cast<Vulkan::Pipeline>(*bound_pipeline));
	}
}

void Renderer::bind_descriptor_sets(Disarray::CommandExecutor& executor, VkPipelineLayout pipeline_layout)
{
	const std::array desc { get_graphics_resource().get_descriptor_set(swapchain.get_current_frame(), 0),
		get_graphics_resource().get_descriptor_set(swapchain.get_current_frame(), 1),
		get_graphics_resource().get_descriptor_set(swapchain.get_current_frame(), 2) };
	vkCmdBindDescriptorSets(supply_cast<Vulkan::CommandExecutor>(executor), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
		static_cast<std::uint32_t>(desc.size()), desc.data(), 0, nullptr);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const GeometryProperties& properties)
{
	draw_mesh(executor, mesh, properties.to_transform());
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const glm::mat4& transform)
{
	draw_mesh(executor, mesh, *mesh.get_properties().pipeline, transform);
}

void Renderer::draw_mesh(
	Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline, const glm::mat4& transform)
{
	draw_mesh(executor, mesh, mesh_pipeline, transform, 0);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const glm::mat4& transform, const std::uint32_t identifier)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, mesh_pipeline);

	auto& pc = get_graphics_resource().get_editable_push_constant();

	pc.object_transform = transform;
	pc.current_identifier = identifier;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

	bind_descriptor_sets(executor, pipeline.get_layout());

	std::array<VkBuffer, 1> arr {};
	arr[0] = supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices());
	std::array<VkDeviceSize, 1> offsets = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(mesh.get_indices()), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(mesh.get_indices().size()), 1, 0, 0, 0);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Texture& texture, const glm::mat4& transform, const std::uint32_t identifier)
{
	draw_mesh(executor, mesh, mesh_pipeline, texture, glm::vec4 { 1.0F }, transform, identifier);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Texture& texture, const glm::vec4& colour, const glm::mat4& transform, const std::uint32_t identifier)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, mesh_pipeline);

	(void)texture;
	auto& pc = get_graphics_resource().get_editable_push_constant();

	pc.object_transform = transform;
	pc.colour = colour;
	pc.current_identifier = identifier;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

	bind_descriptor_sets(executor, pipeline.get_layout());

	std::array<VkBuffer, 1> arr {};
	arr[0] = supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices());
	const std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	const auto& indices = cast_to<Vulkan::IndexBuffer>(mesh.get_indices());
	vkCmdBindIndexBuffer(command_buffer, indices.supply(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Renderer::draw_submesh(Disarray::CommandExecutor& executor, const Disarray::VertexBuffer& vertex_buffer,
	const Disarray::IndexBuffer& index_buffer, const Disarray::Pipeline& mesh_pipeline, const Disarray::Texture& texture, const glm::vec4& colour,
	const glm::mat4& transform, const std::uint32_t identifier)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, mesh_pipeline);

	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	push_constant.object_transform = transform;
	push_constant.colour = colour;
	push_constant.current_identifier = identifier;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	bind_descriptor_sets(executor, pipeline.get_layout());

	std::array<VkBuffer, 1> arr {};
	arr[0] = supply_cast<Vulkan::VertexBuffer>(vertex_buffer);
	const std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(index_buffer), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(index_buffer.size()), 1, 0, 0, 0);
}

void Renderer::draw_submeshes(Disarray::CommandExecutor& executor, const Disarray::Mesh& parent_mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Texture& texture, const glm::vec4& colour, const glm::mat4& transform, const std::uint32_t identifier)
{
	// draw_mesh(executor, parent_mesh, mesh_pipeline, texture, transform, identifier);

	auto& push_constant = get_graphics_resource().get_editable_push_constant();
	for (const auto& sub : parent_mesh.get_submeshes()) {
		auto&& [key, mesh] = sub;

		std::size_t index = 0;
		for (const auto& texture_index : mesh->texture_indices) {
			push_constant.image_indices.at(index++) = static_cast<int>(texture_index);
		}
		push_constant.bound_textures = static_cast<unsigned int>(index);
		draw_submesh(executor, *mesh->vertices, *mesh->indices, mesh_pipeline, texture, colour, transform, identifier);
		push_constant.image_indices.fill(-1);
		push_constant.bound_textures = 0;
	}
}

void Renderer::end_pass(Disarray::CommandExecutor& executor)
{
	submit_batched_geometry(executor);
	vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor));
}

void Renderer::begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& framebuffer, bool explicit_clear)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

	VkRenderPassBeginInfo render_pass_begin_info {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = supply_cast<Vulkan::RenderPass>(framebuffer.get_render_pass());
	render_pass_begin_info.framebuffer = supply_cast<Vulkan::Framebuffer>(framebuffer);
	render_pass_begin_info.renderArea.offset = { 0, 0 };

	VkExtent2D extent_2_d { .width = extent.width, .height = extent.height };
	render_pass_begin_info.renderArea.extent = extent_2_d;

	auto clear_values = cast_to<Vulkan::Framebuffer>(framebuffer).get_clear_values();

	render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_begin_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	if (explicit_clear) {
		auto& vk_framebuffer = cast_to<Vulkan::Framebuffer>(framebuffer);

		std::vector<VkClearValue> fb_clear_values = vk_framebuffer.get_clear_values();

		const std::uint32_t color_attachment_count = vk_framebuffer.get_colour_attachment_count();
		const std::uint32_t total_attachment_count = color_attachment_count + (vk_framebuffer.has_depth() ? 1 : 0);

		std::vector<VkClearAttachment> attachments(total_attachment_count);
		std::vector<VkClearRect> clear_rects(total_attachment_count);
		for (uint32_t i = 0; i < color_attachment_count; i++) {
			attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachments[i].colorAttachment = i;
			attachments[i].clearValue = fb_clear_values[i];

			clear_rects[i].rect.offset = { 0, 0 };
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
	viewport.x = 0.0F;
	viewport.y = 0.0F;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 1.0F;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent_2_d;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

} // namespace Disarray::Vulkan
