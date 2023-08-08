#include "DisarrayPCH.hpp"

#include "vulkan/Renderer.hpp"

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/VertexBuffer.hpp"

#include <array>
#include <core/Clock.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Disarray::Vulkan {

	Renderer::Renderer(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::RendererProperties& properties)
		: device(dev)
		, swapchain(sc)
		, props(properties)
		, pipeline_cache(dev, "Assets/Shaders")
		, texture_cache(dev, "Assets/Textures")
	{
		frame_ubos.resize(swapchain.image_count());
		for (auto& ubo : frame_ubos) {
			ubo = UniformBuffer::construct(device,
				BufferProperties {
					.size = sizeof(UBO),
					.binding = 0,
				});
		}

		initialise_descriptors();

		auto samples = SampleCount::ONE;

		FramebufferProperties geometry_props { .extent = swapchain.get_extent(),
			.attachments = { { ImageFormat::SBGR }, { ImageFormat::Depth } },
			.clear_colour_on_load = false,
			.clear_depth_on_load = false,
			.samples = samples,
			.debug_name = "RendererFramebuffer" };
		geometry_framebuffer = Framebuffer::construct(device, geometry_props);

		auto quad_framebuffer = Framebuffer::construct(device,
			{ .extent = swapchain.get_extent(),
				.attachments = { { ImageFormat::SBGR },  { ImageFormat::Uint, false },{ ImageFormat::Depth }, },
				.samples = samples,
				.debug_name = "QuadFramebuffer" });

		PipelineCacheCreationProperties pipeline_properties = {
			.pipeline_key = "quad",
			.shader_key = "quad",
			.framebuffer = geometry_framebuffer,
			.layout = { LayoutElement { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float2, "normals" },
				{ ElementType::Float4, "colour" }, { ElementType::Uint, "identifier" } },
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, sizeof(PushConstant) } },
			.extent = swapchain.get_extent(),
			.samples = samples,
			.descriptor_set_layout = layouts.data(),
			.descriptor_set_layout_count = static_cast<std::uint32_t>(layouts.size()),
		};
		{
			// Quad
			pipeline_properties.framebuffer = quad_framebuffer;
			pipeline_cache.put(pipeline_properties);
		}
		{
			// Line
			pipeline_properties.framebuffer = geometry_framebuffer;
			pipeline_properties.pipeline_key = "line";
			pipeline_properties.shader_key = "line";
			pipeline_properties.line_width = 8.0f;
			pipeline_properties.polygon_mode = PolygonMode::Line;
			pipeline_properties.layout = { { ElementType::Float3, "pos" }, { ElementType::Float4, "colour" } };
			pipeline_cache.put(pipeline_properties);
		}

		render_batch.construct(*this, device);
	}

	Renderer::~Renderer()
	{
		const auto& vk_device = supply_cast<Vulkan::Device>(device);
		std::for_each(std::begin(layouts), std::end(layouts),
			[&vk_device](VkDescriptorSetLayout& layout) { vkDestroyDescriptorSetLayout(vk_device, layout, nullptr); });

		vkDestroyDescriptorPool(vk_device, pool, nullptr);
		descriptors.clear();
	}

	void Renderer::on_resize() { extent = swapchain.get_extent(); }

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

	void Renderer::end_pass(Disarray::CommandExecutor& executor) { vkCmdEndRenderPass(supply_cast<Vulkan::CommandExecutor>(executor)); }

	void Renderer::begin_frame(Camera& camera)
	{
		// TODO: Move to some kind of scene scope?
		render_batch.reset();

		uniform.view = camera.get_view_matrix();
		uniform.proj = camera.get_projection_matrix();
		uniform.view_projection = camera.get_view_projection();

		auto& current_uniform = frame_ubos[swapchain.get_current_frame()];
		current_uniform->set_data<UBO>(&uniform);

		if (swapchain.needs_recreation()) {
			force_recreation();
		}
	}

	void Renderer::end_frame() { std::memset(&uniform, 0, sizeof(UBO)); }

	void Renderer::force_recreation()
	{
		on_resize();
		// default_framebuffer->force_recreation();
	}

	void Renderer::FrameDescriptor::destroy(Disarray::Device& dev) { }

} // namespace Disarray::Vulkan
