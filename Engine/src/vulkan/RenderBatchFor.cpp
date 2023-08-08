#include "DisarrayPCH.hpp"

#include "core/Types.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/Texture.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

	template <> void RenderBatchFor<QuadVertex, max_objects, vertex_count<QuadVertex>>::construct(Disarray::Renderer& renderer, Disarray::Device& dev)
	{
		const auto& quad_pipeline = renderer.get_pipeline_cache().get("quad");
		pipeline = cast_to<Vulkan::Pipeline>(quad_pipeline);
		std::vector<std::uint32_t> quad_indices;
		quad_indices.resize(vertices.size() * 6);
		std::uint32_t offset = 0;
		for (std::size_t i = 0; i < vertices.size() * 6; i += 6) {
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
				.size = quad_indices.size() * vertex_count<QuadVertex>,
				.count = quad_indices.size(),
			});

		vertex_buffer = make_scope<Vulkan::VertexBuffer>(dev, BufferProperties { .size = buffer_size(), .count = vertices.size() });
	}

	template <>
	void RenderBatchFor<QuadVertex, max_objects, vertex_count<QuadVertex>>::submit(
		Disarray::Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		if (submitted_indices == 0)
			return;

		prepare_data();

		renderer.get_editable_push_constant().max_identifiers = submitted_objects;

		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

		const auto& vb = vertex_buffer;
		const auto& ib = index_buffer;
		// const auto& descriptor = descriptor_sets[renderer.get_current_frame()];
		const auto index_count = submitted_indices;

		const std::array<VkDescriptorSet, 1> desc { renderer.get_descriptor_set() };
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_layout(), 0, 1, desc.data(), 0, nullptr);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->supply());

		vkCmdPushConstants(command_buffer, pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
			renderer.get_push_constant());

		const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
		constexpr VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

		/*
		 * if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}*/

		const auto count = index_count;
		vkCmdDrawIndexed(command_buffer, count, 1, 0, 0, 0);
	}

	template <>
	void RenderBatchFor<QuadVertex, max_objects, vertex_count<QuadVertex>>::create_new(Geometry geom, const Disarray::GeometryProperties& props)
	{
		if (geom != Geometry::Rectangle)
			return;

		static constexpr std::array<glm::vec2, 4> texture_coordinates = { glm::vec2 { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		static constexpr std::array<glm::vec4, 4> quad_positions
			= { glm::vec4 { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, 0.5f, 0.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f } };
		static constexpr glm::vec4 quad_normal = glm::vec4 { 0, 0, 1, 0 };

		glm::mat4 transform
			= glm::translate(glm::mat4 { 1.0f }, props.position) * glm::mat4_cast(props.rotation) * glm::scale(glm::mat4 { 1.0f }, *props.dimensions);

		for (std::size_t i = 0; i < vertex_count<QuadVertex>; i++) {
			auto& vertex = emplace();
			vertex.pos = transform * quad_positions[i];
			vertex.normals = transform * quad_normal;
			vertex.uvs = texture_coordinates[i];
			vertex.colour = props.colour;
			vertex.identifier = *props.identifier;
		}

		submitted_indices += 6;
		submitted_objects++;
	}

	// LINES

	template <> void RenderBatchFor<LineVertex, max_objects, vertex_count<LineVertex>>::construct(Disarray::Renderer& renderer, Disarray::Device& dev)
	{
		pipeline = cast_to<Vulkan::Pipeline>(renderer.get_pipeline_cache().get("line"));

		std::vector<std::uint32_t> line_indices;
		line_indices.resize(vertices.size() * vertex_count<LineVertex>);
		std::uint32_t offset = 0;
		for (std::size_t i = 0; i < vertices.size() * vertex_count<LineVertex>; i += vertex_count<LineVertex>) {
			line_indices[i] = offset;
			line_indices[i + 1] = offset + 1;
			offset += vertex_count<LineVertex>;
		}

		index_buffer = make_scope<Vulkan::IndexBuffer>(dev,
			BufferProperties {
				.data = line_indices.data(),
				.size = line_indices.size() * vertex_count<LineVertex>,
				.count = line_indices.size(),
			});

		vertex_buffer = make_scope<Vulkan::VertexBuffer>(dev, BufferProperties { .size = buffer_size(), .count = vertices.size() });
	}

	template <>
	void RenderBatchFor<LineVertex, max_objects, vertex_count<LineVertex>>::submit(
		Disarray::Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		if (submitted_indices == 0)
			return;

		prepare_data();

		renderer.get_editable_push_constant().max_identifiers = submitted_objects;

		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

		const auto& vb = vertex_buffer;
		const auto& ib = index_buffer;
		// const auto& descriptor = descriptor_sets[renderer.get_current_frame()];
		const auto index_count = submitted_indices;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->supply());

		vkCmdPushConstants(command_buffer, pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
			renderer.get_push_constant());

		const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(*vb) };
		constexpr VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(*ib), 0, VK_INDEX_TYPE_UINT32);

		vkCmdSetLineWidth(command_buffer, pipeline->get_properties().line_width);

		/*
		 * if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}*/

		const auto count = index_count;
		vkCmdDrawIndexed(command_buffer, count, count / 2, 0, 0, 0);
	}

	template <>
	void RenderBatchFor<LineVertex, max_objects, vertex_count<LineVertex>>::create_new(Geometry geom, const Disarray::GeometryProperties& props)
	{
		if (geom != Geometry::Line)
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

} // namespace Disarray
