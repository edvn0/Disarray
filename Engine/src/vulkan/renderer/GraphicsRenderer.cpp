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

	template <std::size_t Vertices> void BatchRenderer<Vertices>::submit(Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		renderer.get_editable_push_constant().max_identifiers = submitted_geometries;
		quads.submit(renderer, command_executor);
		lines.submit(renderer, command_executor);
	}

	void Renderer::draw_mesh(Disarray::CommandExecutor& executor, Disarray::Mesh& mesh, const GeometryProperties& properties)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
		const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh.get_pipeline());
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

		pc.object_transform = properties.to_transform();
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

	void Renderer::submit_batched_geometry(Disarray::CommandExecutor& executor) { render_batch.submit(*this, executor); }

	void Renderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties)
	{
		switch (geometry) {
		case Geometry::Circle:
			break;
		case Geometry::Triangle:
			break;
		case Geometry::Line:
			render_batch.lines.create_new(properties);
			break;
		case Geometry::Rectangle:
			render_batch.quads.create_new(properties);
			break;
		}
		render_batch.submitted_geometries++;
	}

} // namespace Disarray::Vulkan
