#include "glm/ext/matrix_transform.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray::Vulkan {

	template <>
	void RenderBatchFor<QuadVertex, max_vertices, vertex_count<QuadVertex>>::construct(
		Renderer& renderer, Disarray::Device& dev, Disarray::Swapchain& swapchain)
	{
		pipeline = cast_to<Vulkan::Pipeline>(renderer.get_pipeline_cache().get("quad"));
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
		index_buffer = IndexBuffer::construct(dev, swapchain,
			{
				.data = quad_indices.data(),
				.size = quad_indices.size() * vertex_count,
				.count = quad_indices.size(),
			});

		vertex_buffer = VertexBuffer::construct(dev, swapchain, { .size = vertices.size() * vertex_count, .count = vertices.size() });
	}

	template <>
	void RenderBatchFor<QuadVertex, max_vertices, vertex_count<QuadVertex>>::submit(Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		if (submitted_indices == 0)
			return;

		prepare_data();

		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

		const auto& vb = vertex_buffer;
		const auto& ib = index_buffer;
		// const auto& descriptor = descriptor_sets[renderer.get_current_frame()];
		const auto index_count = submitted_indices;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->supply());

		vkCmdPushConstants(command_buffer, pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
			renderer.get_push_constant());

		const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(vb) };
		constexpr VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(ib), 0, VK_INDEX_TYPE_UINT32);

		/*
		 * if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}*/

		const auto count = index_count;
		vkCmdDrawIndexed(command_buffer, count, 1, 0, 0, 0);
	}

	template <> void RenderBatchFor<QuadVertex, max_vertices, vertex_count<QuadVertex>>::create_new(const Disarray::GeometryProperties& props)
	{
		static constexpr std::size_t quad_vertex_count = 4;
		static constexpr std::array<glm::vec2, 4> texture_coordinates = { glm::vec2 { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		static constexpr std::array<glm::vec4, 4> quad_positions
			= { glm::vec4 { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, 0.5f, 0.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f } };
		static constexpr glm::vec4 quad_normal = glm::vec4 { 0, 0, 1, 0 };

		glm::mat4 transform = glm::scale(glm::mat4 { 1.0f }, *props.dimensions) * glm::translate(glm::mat4 { 1.0f }, props.position);

		for (std::size_t i = 0; i < quad_vertex_count; i++) {
			auto& vertex = emplace();
			vertex.pos = transform * quad_positions[i];
			vertex.normals = transform * quad_normal;
			vertex.uvs = texture_coordinates[i];
			vertex.colour = { 1.0f, 0.5f, 0.5f, 1.0f };
		}

		submitted_indices += 6;
	}

	// LINES

	template <>
	void RenderBatchFor<LineVertex, max_vertices, vertex_count<LineVertex>>::construct(
		Renderer& renderer, Disarray::Device& dev, Disarray::Swapchain& swapchain)
	{
		pipeline = cast_to<Vulkan::Pipeline>(renderer.get_pipeline_cache().get("line"));

		std::vector<std::uint32_t> line_indices;
		line_indices.resize(vertices.size() * vertex_count);
		std::uint32_t offset = 0;
		for (std::size_t i = 0; i < vertices.size() * vertex_count; i += vertex_count) {
			line_indices[i] = offset;
			line_indices[i + 1] = offset + 1;
			offset += vertex_count;
		}
		index_buffer = IndexBuffer::construct(dev, swapchain,
			{
				.data = line_indices.data(),
				.size = line_indices.size() * vertex_count,
				.count = line_indices.size(),
			});

		vertex_buffer = VertexBuffer::construct(dev, swapchain, { .size = vertices.size() * vertex_count, .count = vertices.size() });
	}

	template <>
	void RenderBatchFor<LineVertex, max_vertices, vertex_count<LineVertex>>::submit(Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		if (submitted_indices == 0)
			return;

		prepare_data();

		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(command_executor);

		const auto& vb = vertex_buffer;
		const auto& ib = index_buffer;
		// const auto& descriptor = descriptor_sets[renderer.get_current_frame()];
		const auto& pipeline = cast_to<Vulkan::Pipeline>(renderer.get_pipeline_cache().get("line"));
		const auto index_count = submitted_indices;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->supply());

		vkCmdPushConstants(command_buffer, pipeline->get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant),
			renderer.get_push_constant());

		const std::array<VkBuffer, 1> vbs { supply_cast<Vulkan::VertexBuffer>(vb) };
		constexpr VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(ib), 0, VK_INDEX_TYPE_UINT32);

		/*
		 * if (pipeline->get_vulkan_pipeline_layout()) {
			vkCmdBindDescriptorSets(
				command_buffer.get_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_vulkan_pipeline_layout(), 0, 1, &descriptor, 0, nullptr);
		}*/

		const auto count = index_count;
		vkCmdDrawIndexed(command_buffer, count, 1, 0, 0, 0);
	}

	template <> void RenderBatchFor<LineVertex, max_vertices, vertex_count<LineVertex>>::create_new(const Disarray::GeometryProperties& props)
	{
		{
			auto& vertex = emplace();
			vertex.pos = props.position;
			vertex.colour = { 0, 1, 0, 1 };
		}

		{
			auto& vertex = emplace();
			vertex.pos = props.to_position;
			vertex.colour = { 0, 1, 0, 1 };
		}

		submitted_indices += 2;
	}
} // namespace Disarray::Vulkan