#include "DisarrayPCH.hpp"

// clang-format off
#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/Renderer.hpp"
// clang-format on

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>

#include "core/Instrumentation.hpp"
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
	if (bound_pipeline == nullptr || &pipeline != bound_pipeline) {
		bound_pipeline = &pipeline;
	}

	if (bound_pipeline != nullptr) {
		vkCmdBindPipeline(
			supply_cast<Vulkan::CommandExecutor>(executor), static_cast<VkPipelineBindPoint>(point), supply_cast<Vulkan::Pipeline>(*bound_pipeline));
	}
}

void Renderer::bind_descriptor_sets(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline)
{
	auto* pipeline_layout = cast_to<Vulkan::Pipeline>(pipeline).get_layout();
	FrameIndex index { swapchain.get_current_frame() };
	const std::array desc { get_graphics_resource().get_descriptor_set(index, DescriptorSet(0)),
		get_graphics_resource().get_descriptor_set(index, DescriptorSet(1)), get_graphics_resource().get_descriptor_set(index, DescriptorSet(2)),
		get_graphics_resource().get_descriptor_set(index, DescriptorSet(3)) };
	vkCmdBindDescriptorSets(supply_cast<Vulkan::CommandExecutor>(executor), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
		static_cast<std::uint32_t>(desc.size()), desc.data(), 0, nullptr);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const GeometryProperties& properties)
{
	draw_mesh(executor, mesh, properties.to_transform());
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const glm::mat4& transform)
{
	ensure(false, "Never call this!");
}

void Renderer::draw_mesh(
	Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline, const glm::mat4& transform)
{
	draw_mesh(executor, mesh, mesh_pipeline, transform, 0);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const glm::mat4& transform, const std::uint32_t identifier)
{
	if (mesh.invalid()) {
		return;
	}

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, mesh_pipeline);

	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	push_constant.object_transform = transform;
	push_constant.current_identifier = identifier;
	push_constant.colour = { 1, 1, 1, 1 };
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	bind_descriptor_sets(executor, pipeline);

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

void Renderer::draw_mesh_instanced(Disarray::CommandExecutor& executor, std::size_t instance_count, const Disarray::VertexBuffer& vertex_buffer,
	const Disarray::IndexBuffer& index_buffer, const Disarray::Pipeline& mesh_pipeline)
{
	if (vertex_buffer.invalid() || index_buffer.invalid()) {
		return;
	}

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, pipeline);
	bind_descriptor_sets(executor, pipeline);

	std::array arr { supply_cast<Vulkan::VertexBuffer>(vertex_buffer) };
	std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(index_buffer), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(index_buffer.size()), static_cast<std::uint32_t>(instance_count), 0, 0, 0);
}

void Renderer::draw_mesh_instanced(Disarray::CommandExecutor& executor, std::size_t instance_count, const Disarray::Pipeline& mesh_pipeline)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, pipeline);
	bind_descriptor_sets(executor, pipeline);

	std::array arr { supply_cast<Vulkan::VertexBuffer>(aabb_model->get_vertices()) };
	std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(aabb_model->get_indices()), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(
		command_buffer, static_cast<std::uint32_t>(aabb_model->get_indices().size()), static_cast<std::uint32_t>(instance_count), 0, 0, 0);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Texture& texture, const glm::mat4& transform, const std::uint32_t identifier)
{
	draw_mesh(executor, mesh, mesh_pipeline, texture, glm::vec4 { 1.0F }, transform, identifier);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Texture& texture, const glm::vec4& colour, const glm::mat4& transform, const std::uint32_t identifier)
{
	if (mesh.invalid()) {
		return;
	}

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, pipeline);

	(void)texture;
	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	push_constant.object_transform = transform;
	push_constant.colour = colour;
	push_constant.current_identifier = identifier;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	bind_descriptor_sets(executor, pipeline);

	const std::array arr { supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices()) };
	const std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	const auto& indices = cast_to<Vulkan::IndexBuffer>(mesh.get_indices());
	vkCmdBindIndexBuffer(command_buffer, indices.supply(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const glm::vec4& colour, const glm::mat4& transform)
{
	if (mesh.invalid()) {
		return;
	}

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, pipeline);

	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	push_constant.object_transform = transform;
	push_constant.colour = colour;
	push_constant.current_identifier = 0;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	bind_descriptor_sets(executor, pipeline);

	const std::array arr { supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices()) };
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
	const glm::mat4& transform, const std::uint32_t identifier, PushConstant& push_constant)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);

	push_constant.object_transform = transform;
	push_constant.colour = colour;
	push_constant.current_identifier = identifier;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	std::array<VkBuffer, 1> arr {};
	arr[0] = supply_cast<Vulkan::VertexBuffer>(vertex_buffer);
	const std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(index_buffer), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(index_buffer.size()), 1, 0, 0, 0);
}

void Renderer::draw_submeshes(Disarray::CommandExecutor& executor, const Disarray::Mesh& parent_mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Texture& texture, const glm::vec4& colour, const glm::mat4& transform, const std::uint32_t identifier)
{
	if (parent_mesh.invalid()) {
		return;
	}

	// draw_mesh(executor, parent_mesh, mesh_pipeline, texture, transform, identifier);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_descriptor_sets(executor, pipeline);
	bind_pipeline(executor, pipeline);

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(supply_cast<Vulkan::CommandExecutor>(executor), pipeline.get_properties().line_width);
	}

	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	Collections::for_each(parent_mesh.get_submeshes(), [&](const auto& sub) {
		auto&& [key, mesh] = sub;

		std::size_t index = 0;
		for (const auto& texture_index : mesh->texture_indices) {
			push_constant.image_indices.at(index++) = static_cast<int>(texture_index);
		}
		push_constant.bound_textures = static_cast<unsigned int>(index);
		draw_submesh(executor, *mesh->vertices, *mesh->indices, mesh_pipeline, texture, colour, transform, identifier, push_constant);
		push_constant.bound_textures = 0;
	});
}

void Renderer::draw_aabb(Disarray::CommandExecutor& executor, const Disarray::AABB& aabb, const glm::vec4& colour, const glm::mat4& transform)
{
	const auto scale_matrix = aabb.calculate_scale_matrix();
	// Calculate the center of the AABB
	glm::vec3 aabb_center = aabb.middle_point();
	glm::vec3 translation = -aabb_center;

	glm::mat4 transformation_matrix = glm::mat4(1.0F);
	transformation_matrix = glm::translate(transformation_matrix, translation);
	transformation_matrix = transform * scale_matrix * transformation_matrix;
	draw_mesh(executor, *aabb_model, *aabb_pipeline, colour, transformation_matrix);
}

void Renderer::draw_identifier(
	Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipe, std::uint32_t identifier, const glm::mat4& transform)
{
	const auto& pipeline = cast_to<Vulkan::Pipeline>(pipe);
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	push_constant.object_transform = transform;
	push_constant.current_identifier = identifier;
	push_constant.colour = { 1, 1, 1, 1 };
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	std::array<VkBuffer, 1> arr {};
	arr[0] = supply_cast<Vulkan::VertexBuffer>(aabb_model->get_vertices());
	std::array<VkDeviceSize, 1> offsets = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(aabb_model->get_indices()), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(aabb_model->get_indices().size()), 1, 0, 0, 0);
}

void Renderer::text_rendering_pass(Disarray::CommandExecutor& executor) { text_renderer.render(*this, executor); }

void Renderer::planar_geometry_pass(Disarray::CommandExecutor& executor) { batch_renderer.submit(*this, executor); }

void Renderer::fullscreen_quad_pass(Disarray::CommandExecutor& executor, const Extent&)
{
	auto& framebuffer = fullscreen_quad_pipeline->get_framebuffer();
	begin_pass(executor, framebuffer, false, RenderAreaExtent { framebuffer });

	auto* cmd = supply_cast<Vulkan::CommandExecutor>(executor);
	bind_pipeline(executor, *fullscreen_quad_pipeline);
	vkCmdDrawIndexed(cmd, 3, 1, 0, 0, 0);

	end_pass(executor);
}

void Renderer::end_pass(Disarray::CommandExecutor& executor)
{
	vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor));
	batch_renderer.reset();
}

void Renderer::begin_pass(
	Disarray::CommandExecutor& executor, Disarray::Framebuffer& framebuffer, bool explicit_clear, const RenderAreaExtent& render_area_extent)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

	VkRenderPassBeginInfo render_pass_begin_info {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = supply_cast<Vulkan::RenderPass>(framebuffer.get_render_pass());
	render_pass_begin_info.framebuffer = supply_cast<Vulkan::Framebuffer>(framebuffer);
	render_pass_begin_info.renderArea.offset = {
		static_cast<std::int32_t>(render_area_extent.offset.width),
		static_cast<std::int32_t>(render_area_extent.offset.height),
	};

	VkExtent2D extent_2_d {
		.width = render_area_extent.extent.width,
		.height = render_area_extent.extent.height,
	};
	render_pass_begin_info.renderArea.extent = extent_2_d;

	const auto& clear_values = cast_to<Vulkan::Framebuffer>(framebuffer).get_clear_values();

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

	set_viewport(executor,
		{
			extent_2_d.width,
			extent_2_d.height,
		});
	set_scissors(executor,
		{
			render_area_extent.extent.width,
			render_area_extent.extent.height,
		},
		{
			render_area_extent.offset.width,
			render_area_extent.offset.height,
		});
}

void Renderer::set_scissors(Disarray::CommandExecutor& executor, const glm::vec2& scissor_extent, const glm::vec2& offset)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

	VkRect2D scissor {};
	scissor.offset = {
		static_cast<std::int32_t>(offset.x),
		static_cast<std::int32_t>(offset.y),
	};
	scissor.extent = {
		.width = static_cast<std::uint32_t>(scissor_extent.x),
		.height = static_cast<std::uint32_t>(scissor_extent.y),
	};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void Renderer::set_viewport(Disarray::CommandExecutor& executor, const glm::vec2& viewport_extent)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

	VkViewport viewport {};
	viewport.x = 0.0F;
	viewport.y = 0.0F;
	viewport.width = static_cast<float>(viewport_extent.x);
	viewport.height = static_cast<float>(viewport_extent.y);
	viewport.minDepth = 0.0F;
	viewport.maxDepth = 1.0F;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
}

} // namespace Disarray::Vulkan
