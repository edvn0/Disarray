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

	void Renderer::submit_batched_geometry(Disarray::CommandExecutor& executor)
	{
		render_batch.submit(*this, executor);
		render_batch.reset();
	}

	void Renderer::on_batch_full(std::function<void(Disarray::Renderer&)>&& func) { on_batch_full_func = func; }

	void Renderer::flush_batch(Disarray::CommandExecutor& executor)
	{
		render_batch.submit(*this, executor);
		render_batch.reset();
	}

	void Renderer::draw_planar_geometry(Geometry geometry, const GeometryProperties& properties)
	{
		add_geometry_to_batch(geometry, properties);

		if (render_batch.is_full()) {
			on_batch_full_func(*this);
		}
	}

	void Renderer::add_geometry_to_batch(Disarray::Geometry geometry, const Disarray::GeometryProperties& properties)
	{
		render_batch.create_new(geometry, properties);
		render_batch.submitted_geometries++;
	}

} // namespace Disarray::Vulkan
