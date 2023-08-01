#include "vulkan/Renderer.hpp"

#include "core/Types.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
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

	template <std::size_t Vertices> void RenderBatch<Vertices>::submit(Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		quads.submit(renderer, command_executor);
		lines.submit(renderer, command_executor);
	}

	Renderer::Renderer(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::RendererProperties& properties)
		: device(dev)
		, swapchain(sc)
		, props(properties)
	{
		pipeline_cache = make_ref<PipelineCache>(device, swapchain, "Assets/Shaders");
		default_framebuffer = Framebuffer::construct(device, swapchain, { .debug_name = "RendererFramebuffer" });

		PipelineCacheCreationProperties pipeline_properties = {
			.pipeline_key = "quad",
			.shader_key = "quad",
			.framebuffer = default_framebuffer,
			.layout = { LayoutElement { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float2, "normals" },
				{ ElementType::Float4, "colour" } },
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 80 } } },
			.extent = swapchain.get_extent(),
		};
		{
			// Quad
			pipeline_cache->put(pipeline_properties);
		}
		{
			// Line
			pipeline_properties.pipeline_key = "line";
			pipeline_properties.shader_key = "line";
			pipeline_properties.line_width = 5.0f;
			pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" } };
			pipeline_cache->put(pipeline_properties);
		}

		render_batch.quads.construct(*this, device, swapchain);
		render_batch.lines.construct(*this, device, swapchain);
	}

	Renderer::~Renderer() { }

	void Renderer::set_extent(const Extent& e) { extent = e; }

	void Renderer::begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& fb)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

		VkRenderPassBeginInfo render_pass_begin_info {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = supply_cast<Vulkan::RenderPass>(fb.get_render_pass());
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

	void Renderer::end_pass(Disarray::CommandExecutor& executor) { vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor)); }

	void Renderer::draw_mesh(Disarray::CommandExecutor& executor, Disarray::Mesh& mesh)
	{
		auto command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
		const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh.get_pipeline());
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

		vkCmdPushConstants(
			command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &pc);

		std::array<VkBuffer, 1> arr;
		arr[0] = supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices());
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets);

		vkCmdBindIndexBuffer(command_buffer, supply_cast<Vulkan::IndexBuffer>(mesh.get_indices()), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, mesh.get_indices().size(), 1, 0, 0, 0);
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
	}

	void Renderer::begin_frame(UsageBadge<App>)
	{
		static glm::vec3 rotation { 90.f, 0.f, 0.f };
		const auto rotate = glm::rotate(glm::mat4 { 1.0f }, glm::radians(rotation.z), glm::vec3 { 0.f, 0.f, 1.f })
			* glm::rotate(glm::mat4 { 1.0f }, glm::radians(rotation.y), glm::vec3 { 0.f, 1.f, 0.f })
			* glm::rotate(glm::mat4 { 1.0f }, glm::radians(rotation.x), glm::vec3 { 1.f, 0.f, 0.f });
		pc = { .object_transform = rotate, .colour = glm::vec4 { 1, 1, 0, 1 } };
		rotation += 0.1;
		// TODO: Move to some kind of scene scope?
		render_batch.reset();

		if (swapchain.needs_recreation()) {
			force_recreation();
		}
	}

	void Renderer::end_frame(UsageBadge<Disarray::App>) { }

	void Renderer::force_recreation() { default_framebuffer->force_recreation(); }

} // namespace Disarray::Vulkan