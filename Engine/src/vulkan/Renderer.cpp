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
#include "vulkan/vulkan_core.h"

#include <array>
#include <core/Clock.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Disarray::Vulkan {

	template <std::size_t Vertices> void BatchRenderer<Vertices>::submit(Renderer& renderer, Disarray::CommandExecutor& command_executor)
	{
		quads.submit(renderer, command_executor);
		lines.submit(renderer, command_executor);
	}

	Renderer::Renderer(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::RendererProperties& properties)
		: device(dev)
		, swapchain(sc)
		, props(properties)
	{
		frame_ubos.resize(swapchain.image_count());
		for (auto& ubo : frame_ubos) {
			ubo = UniformBuffer::construct(device, swapchain,
				BufferProperties {
					.size = sizeof(UBO),
					.binding = 0,
				});
		}

		initialise_descriptors();

		pipeline_cache = make_scope<PipelineCache>(device, swapchain, "Assets/Shaders");
		auto samples = SampleCount::ONE;
		geometry_framebuffer = Framebuffer::construct(device, swapchain, { .samples = samples, .debug_name = "RendererFramebuffer" });

		const std::array<VkDescriptorSetLayout, 1> desc_layout { layout };

		PipelineCacheCreationProperties pipeline_properties = {
			.pipeline_key = "quad",
			.shader_key = "quad",
			.framebuffer = geometry_framebuffer,
			.layout = { LayoutElement { ElementType::Float3, "position" }, { ElementType::Float2, "uvs" }, { ElementType::Float2, "normals" },
				{ ElementType::Float4, "colour" } },
			.push_constant_layout = PushConstantLayout { PushConstantRange { PushConstantKind::Both, std::size_t { 80 } } },
			.extent = swapchain.get_extent(),
			.samples = samples,
			.descriptor_set_layout = desc_layout.data(),
			.descriptor_set_layout_count = static_cast<std::uint32_t>(desc_layout.size()),
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

	void Renderer::on_resize() { extent = swapchain.get_extent(); }

	void Renderer::expose_to_shaders(Disarray::Image& image)
	{
		const auto& descriptor_info = cast_to<Vulkan::Image>(image).get_descriptor_info();
		// Check if we can just add it to the descriptor sets

		// If not, reallocate
		// Else, add it
		// update descriptor sets?
	}

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

		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clear_values[1].depthStencil = { 1.0f, 0 };

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
	}

	void Renderer::begin_frame(UsageBadge<App>)
	{
		// TODO: Move to some kind of scene scope?
		render_batch.reset();

		uniform.view = glm::lookAt(glm::vec3(0, 2, 2), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		uniform.proj = glm::perspective(glm::radians(72.0f), extent.aspect_ratio(), 0.1f, 1000.0f);
		uniform.view_projection = uniform.proj * uniform.view;

		auto& current_uniform = frame_ubos[swapchain.get_current_frame()];
		current_uniform->set_data<UBO>(&uniform);

		if (swapchain.needs_recreation()) {
			force_recreation();
		}
	}

	void Renderer::end_frame(UsageBadge<Disarray::App>) { }

	void Renderer::force_recreation()
	{
		on_resize();
		// default_framebuffer->force_recreation();
	}

	void Renderer::initialise_descriptors()
	{
		auto vk_device = supply_cast<Vulkan::Device>(device);

		TextureProperties texture_properties { .debug_name = "viking" };
		texture_properties.path = "Assets/Textures/viking_room.png";
		texture_properties.format = ImageFormat::SRGB;
		auto viking_room = texture_cache.emplace_back(Texture::construct(device, swapchain, texture_properties));

		std::array<VkDescriptorSetLayoutBinding, 2> bindings {};
		{
			auto binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
			binding.descriptorCount = 1;
			binding.binding = 0;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			bindings[0] = binding;
		}
		{
			auto binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
			binding.descriptorCount = 1;
			binding.binding = 1;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings[1] = binding;
		}

		auto layout_create_info = vk_structures<VkDescriptorSetLayoutCreateInfo> {}();
		layout_create_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
		layout_create_info.pBindings = bindings.data();

		verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &layout));

		auto frames = swapchain.image_count();

		std::array<VkDescriptorPoolSize, 2> sizes;
		sizes[0] = vk_structures<VkDescriptorPoolSize> {}(frames, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		sizes[1] = vk_structures<VkDescriptorPoolSize> {}(frames, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		auto pool_create_info = vk_structures<VkDescriptorPoolCreateInfo> {}();
		pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_create_info.poolSizeCount = static_cast<std::uint32_t>(sizes.size());
		pool_create_info.pPoolSizes = sizes.data();
		pool_create_info.maxSets = static_cast<std::uint32_t>(frames * sizes.size());

		verify(vkCreateDescriptorPool(vk_device, &pool_create_info, nullptr, &pool));

		std::vector<VkDescriptorSetLayout> layouts(swapchain.image_count(), layout);
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain.image_count());
		alloc_info.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> sets(swapchain.image_count());
		descriptors.resize(swapchain.image_count());
		std::size_t i = 0;
		vkAllocateDescriptorSets(vk_device, &alloc_info, sets.data());
		for (auto& descriptor : descriptors) {
			descriptor.set = sets[i];

			VkDescriptorBufferInfo buffer_info {};
			buffer_info.buffer = cast_to<Vulkan::UniformBuffer>(frame_ubos[i])->supply();
			buffer_info.offset = 0;
			buffer_info.range = sizeof(UBO);

			auto write_sets = vk_structures<VkWriteDescriptorSet, 2> {}.multiple();
			write_sets[0].dstSet = descriptors[i].set;
			write_sets[0].dstBinding = 0;
			write_sets[0].dstArrayElement = 0;
			write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_sets[0].descriptorCount = 1;
			write_sets[0].pBufferInfo = &buffer_info;

			VkDescriptorImageInfo image_info = cast_to<Vulkan::Image>(viking_room->get_image()).get_descriptor_info();
			write_sets[1].dstSet = descriptors[i].set;
			write_sets[1].dstBinding = 1;
			write_sets[1].dstArrayElement = 0;
			write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_sets[1].descriptorCount = 1;
			write_sets[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(vk_device, static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
			i++;
		}
	}

	VkDescriptorSet Renderer::get_descriptor_set(std::uint32_t index) { return descriptors[index].set; }

	void Renderer::FrameDescriptor::destroy(Disarray::Device& dev) { }

} // namespace Disarray::Vulkan
