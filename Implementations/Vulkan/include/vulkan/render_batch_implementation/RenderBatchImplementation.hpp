#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#include <array>

#include "core/Clock.hpp"
#include "core/Formatters.hpp"
#include "core/Types.hpp"
#include "graphics/Maths.hpp"
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

template <std::size_t Objects> void QuadVertexBatch<Objects>::construct_impl(Renderer& renderer, const Device& dev)
{
	reset();

	pipeline = renderer.get_pipeline_cache().get("quad");
	std::vector<std::uint32_t> quad_indices;
	quad_indices.resize(vertices.size() * index_per_object_count<QuadVertex>);
	std::uint32_t offset = 0;
	for (std::size_t i = 0; i < vertices.size() * index_per_object_count<QuadVertex>; i += index_per_object_count<QuadVertex>) {
		quad_indices[i + 0] = 0 + offset;
		quad_indices[i + 1] = 1 + offset;
		quad_indices[i + 2] = 2 + offset;
		quad_indices[i + 3] = 2 + offset;
		quad_indices[i + 4] = 3 + offset;
		quad_indices[i + 5] = 0 + offset;
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
		});
}

template <std::size_t Objects> void QuadVertexBatch<Objects>::create_new_impl(Geometry geometry, const GeometryProperties& props)
{
	if (geometry != Geometry::Rectangle)
		return;

	static constexpr std::array<glm::vec2, 4> texture_coordinates = { glm::vec2 { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
	static constexpr std::array<glm::vec4, 4> quad_positions
		= { glm::vec4 { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, 0.5f, 0.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f } };

	glm::mat4 transform
		= glm::translate(glm::mat4 { 1.0f }, props.position) * glm::mat4_cast(props.rotation) * glm::scale(glm::mat4 { 1.0f }, props.dimensions);

	auto start_index = submitted_vertices;
	for (std::size_t i = 0; i < vertex_per_object_count<QuadVertex>; i++) {
		QuadVertex& vertex = emplace();
		vertex.pos = transform * quad_positions[i];
		vertex.uvs = texture_coordinates[i];
		vertex.colour = props.colour;
		vertex.identifier = props.identifier.value_or(0);
	}

	glm::vec3 normals = Maths::compute_normal(vertices[start_index + 1].pos, vertices[start_index + 0].pos, vertices[start_index + 2].pos);

	for (std::size_t i = start_index; i < start_index + vertex_per_object_count<QuadVertex>; i++) {
		vertices[i].normals = normals;
	}

	submitted_indices += 6;
	submitted_objects++;
}

template <std::size_t Objects> void QuadVertexBatch<Objects>::submit_impl(Renderer& renderer, CommandExecutor& command_executor)
{
	if (submitted_indices == 0)
		return;

	prepare_data();

	renderer.get_editable_push_constant().max_identifiers = submitted_objects;

	auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

	const auto& vb = vertex_buffer;
	const auto& ib = index_buffer;
	const auto index_count = submitted_indices;
	const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*pipeline);

	const std::array<VkDescriptorSet, 1> desc { renderer.get_descriptor_set() };
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.get_layout(), 0, 1, desc.data(), 0, nullptr);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.supply());

	vkCmdPushConstants(command_buffer, vk_pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
		renderer.get_push_constant());

	const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
	constexpr VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

	const auto count = index_count;
	vkCmdDrawIndexed(command_buffer, count, 1, 0, 0, 0);
}

template <std::size_t Objects> void QuadVertexBatch<Objects>::flush_impl(Renderer& renderer, CommandExecutor& executor)
{
	submit_impl(renderer, executor);
	reset();
	// flush_vertex_buffer();
}

template <std::size_t Objects> void LineVertexBatch<Objects>::construct_impl(Disarray::Renderer& renderer, const Disarray::Device& dev)
{
	reset();

	pipeline = renderer.get_pipeline_cache().get("line");

	std::vector<std::uint32_t> line_indices;
	line_indices.resize(Objects * IndexCount);
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

template <std::size_t Objects> void LineVertexBatch<Objects>::create_new_impl(Geometry geometry, const Disarray::GeometryProperties& props)
{
	if (geometry != Geometry::Line)
		return;

	if (props.identifier)
		return;

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

template <std::size_t Objects> void LineVertexBatch<Objects>::submit_impl(Disarray::Renderer& renderer, Disarray::CommandExecutor& command_executor)
{
	if (submitted_indices == 0)
		return;

	prepare_data();

	renderer.get_editable_push_constant().max_identifiers = this->submitted_objects;

	auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

	const auto& vb = vertex_buffer;
	const auto& ib = index_buffer;
	const auto index_count = submitted_indices;
	const auto& vk_pipeline = cast_to<Vulkan::Pipeline>(*pipeline);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.supply());

	vkCmdPushConstants(command_buffer, vk_pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
		renderer.get_push_constant());

	const std::array<VkDescriptorSet, 1> desc { renderer.get_descriptor_set() };
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline.get_layout(), 0, 1, desc.data(), 0, nullptr);

	const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
	const VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

	vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

	vkCmdSetLineWidth(command_buffer, vk_pipeline.get_properties().line_width);

	vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
}

template <std::size_t Objects> void LineVertexBatch<Objects>::flush_impl(Disarray::Renderer& renderer, Disarray::CommandExecutor& executor)
{
	submit_impl(renderer, executor);
	reset();
	// flush_vertex_buffer();
}

// LINE ID VERTEX

template <std::size_t Objects> void LineIdVertexBatch<Objects>::construct_impl(Disarray::Renderer& renderer, const Disarray::Device& dev)
{
	this->reset();

	this->pipeline = renderer.get_pipeline_cache().get("line_id");

	std::vector<std::uint32_t> line_indices;
	line_indices.resize(Objects * this->IndexCount);
	for (std::size_t i = 0; i < line_indices.size(); i++) {
		line_indices[i] = static_cast<std::uint32_t>(i);
	}

	this->index_buffer = make_scope<Vulkan::IndexBuffer>(dev,
		BufferProperties {
			.data = line_indices.data(),
			.size = this->index_buffer_size(),
			.count = line_indices.size(),
		});

	this->vertex_buffer = make_scope<Vulkan::VertexBuffer>(dev,
		BufferProperties {
			.size = this->vertex_buffer_size(),
			.count = this->vertices.size(),
		});
}

template <std::size_t Objects> void LineIdVertexBatch<Objects>::create_new_impl(Geometry geometry, const Disarray::GeometryProperties& props)
{
	if (geometry != Geometry::Line)
		return;

	if (!props.identifier)
		return;

	{
		LineIdVertex& vertex = this->emplace();
		vertex.pos = props.position;
		vertex.colour = props.colour;
		vertex.identifier = props.identifier.value_or(0);
	}

	{
		LineIdVertex& vertex = this->emplace();
		vertex.pos = props.to_position;
		vertex.colour = props.colour;
		vertex.identifier = props.identifier.value_or(0);
	}

	this->submitted_indices += 2;
	this->submitted_objects++;
}

} // namespace Disarray