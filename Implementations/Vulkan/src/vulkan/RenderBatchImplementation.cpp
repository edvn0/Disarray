#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#include <array>

#include "core/Clock.hpp"
#include "core/Formatters.hpp"
#include "core/Log.hpp"
#include "core/Types.hpp"
#include "graphics/Maths.hpp"
#include "graphics/RenderBatch.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

void QuadVertexBatch::construct_impl(Renderer& renderer, const Device& dev)
{
	reset();

	pipeline = renderer.get_pipeline_cache().get("Quad");
	std::array<std::uint32_t, BatchRenderer::Objects * IndexCount> quad_indices {};
	std::uint32_t offset = 0;
	for (std::size_t i = 0; i < quad_indices.size(); i += index_per_object_count<QuadVertex>) {
		quad_indices.at(i + 0) = 0 + offset;
		quad_indices.at(i + 1) = 1 + offset;
		quad_indices.at(i + 2) = 2 + offset;
		quad_indices.at(i + 3) = 2 + offset;
		quad_indices.at(i + 4) = 3 + offset;
		quad_indices.at(i + 5) = 0 + offset;
		offset += 4;
	}
	index_buffer = make_scope<Vulkan::IndexBuffer>(dev,
		BufferProperties {
			.data = quad_indices.data(),
			.size = index_buffer_size(),
			.count = quad_indices.size(),
		});

	vertex_buffer = make_scope<Vulkan::VertexBuffer>(dev,
		BufferProperties {
			.size = vertex_buffer_size(),
			.count = vertices.size(),
			.always_mapped = true,
		});
}

void QuadVertexBatch::create_new_impl(Geometry geometry, const GeometryProperties& props)
{
	if (geometry != Geometry::Rectangle) {
		return;
	}

	static constexpr std::array<glm::vec2, 4> texture_coordinates = { glm::vec2 { 0.0F, 0.0F }, { 1.0F, 0.0F }, { 1.0F, 1.0F }, { 0.0F, 1.0F } };
	static constexpr std::array<glm::vec4, 4> quad_positions
		= { glm::vec4 { -0.5F, -0.5F, 0.0F, 1.0F }, { 0.5F, -0.5F, 0.0F, 1.0F }, { 0.5F, 0.5F, 0.0F, 1.0F }, { -0.5F, 0.5F, 0.0F, 1.0F } };

	glm::mat4 transform
		= glm::translate(glm::mat4 { 1.0F }, props.position) * glm::mat4_cast(props.rotation) * glm::scale(glm::mat4 { 1.0F }, props.dimensions);

	for (std::size_t i = 0; i < vertex_per_object_count<QuadVertex>; i++) {
		QuadVertex& vertex = emplace();
		vertex.pos = transform * quad_positions.at(i);
		vertex.uvs = texture_coordinates.at(i);
		vertex.colour = props.colour;
		vertex.normals = glm::vec3 { 0, -1, 0 };
	}

	submitted_indices += index_per_object_count<QuadVertex>;
	submitted_objects++;
}

void QuadVertexBatch::submit_impl(Renderer& renderer, CommandExecutor& command_executor)
{
	if (submitted_indices == 0) {
		return;
	}

	prepare_data();

	auto& resources = renderer.get_graphics_resource();

	resources.get_editable_push_constant().max_identifiers = submitted_objects;

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

	const auto& vb = vertex_buffer;
	const auto& ib = index_buffer;
	const auto index_count = submitted_indices;
	const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*pipeline);

	const std::array<VkDescriptorSet, 1> desc { resources.get_descriptor_set() };
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.get_layout(), 0, 1, desc.data(), 0, nullptr);

	renderer.bind_pipeline(command_executor, *pipeline);

	vkCmdPushConstants(command_buffer, vk_pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
		resources.get_push_constant());

	const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
	constexpr VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

	const auto count = index_count;
	vkCmdDrawIndexed(command_buffer, count, 1, 0, 0, 0);
}

void QuadVertexBatch::flush_impl(Renderer& renderer, CommandExecutor& executor)
{
	submit_impl(renderer, executor);
	reset();
	// flush_vertex_buffer();
}

void QuadVertexBatch::clear_pass_impl(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	renderer.begin_pass(executor, pipeline->get_framebuffer(), true);
	renderer.end_pass(executor);
}

void LineVertexBatch::construct_impl(Disarray::Renderer& renderer, const Disarray::Device& dev)
{
	reset();

	pipeline = renderer.get_pipeline_cache().get("Line");

	std::vector<std::uint32_t> line_indices;
	line_indices.resize(BatchRenderer::Objects * IndexCount);
	for (std::size_t i = 0; i < line_indices.size(); i++) {
		line_indices[i] = static_cast<std::uint32_t>(i);
	}

	index_buffer = make_scope<Vulkan::IndexBuffer>(dev,
		BufferProperties {
			.data = line_indices.data(),
			.size = index_buffer_size(),
			.count = line_indices.size(),
		});

	vertex_buffer = make_scope<Vulkan::VertexBuffer>(dev,
		BufferProperties {
			.size = vertex_buffer_size(),
			.count = vertices.size(),
		});
}

void LineVertexBatch::create_new_impl(Geometry geometry, const Disarray::GeometryProperties& props)
{
	if (geometry != Geometry::Line) {
		return;
	}

	{
		auto& vertex = emplace();
		vertex.pos = props.position;
		vertex.colour = props.colour;
	}

	{
		auto& vertex = emplace();
		vertex.pos = props.to_position;
		vertex.colour = props.colour;
	}

	submitted_indices += 2;
	submitted_objects++;
}

void LineVertexBatch::submit_impl(Disarray::Renderer& renderer, Disarray::CommandExecutor& command_executor)
{
	if (submitted_indices == 0) {
		return;
	}

	prepare_data();

	auto& resources = renderer.get_graphics_resource();

	resources.get_editable_push_constant().max_identifiers = this->submitted_objects;

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

	const auto& vb = vertex_buffer;
	const auto& ib = index_buffer;
	const auto index_count = submitted_indices;
	const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*pipeline);

	renderer.bind_pipeline(command_executor, *pipeline);

	vkCmdPushConstants(command_buffer, vk_pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
		resources.get_push_constant());

	const std::array<VkDescriptorSet, 1> desc { resources.get_descriptor_set() };
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.get_layout(), 0, 1, desc.data(), 0, nullptr);

	const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
	const VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

	vkCmdSetLineWidth(command_buffer, vk_pipeline.get_properties().line_width);

	vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
}

void LineVertexBatch::flush_impl(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	submit_impl(renderer, executor);
	reset();
	// flush_vertex_buffer();
}

void LineVertexBatch::clear_pass_impl(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	renderer.begin_pass(executor, pipeline->get_framebuffer(), true);
	renderer.end_pass(executor);
}

} // namespace Disarray
