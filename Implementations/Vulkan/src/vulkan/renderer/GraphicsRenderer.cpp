#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>

#include "core/Instrumentation.hpp"
#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/MaterialTable.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "util/BitCast.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Material.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/StorageBuffer.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray::Vulkan {

/*
static const std::vector<std::vector<VkWriteDescriptorSet>>& RT_RetrieveOrCreateUniformBufferWriteDescriptors(
	Ref<Vulkan::Material> material, const BufferSet<Disarray::UniformBuffer>* ubo_set)
{
	// UniformBufferSet -> Shader Hash -> Frame -> WriteDescriptor
	std::unordered_map<BufferSet<Disarray::UniformBuffer>*, std::unordered_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>>
		UniformBufferWriteDescriptorCache;

	size_t shaderHash = material->GetShader()->GetHash();
	if (UniformBufferWriteDescriptorCache.find(&ubo_set) != UniformBufferWriteDescriptorCache.end()) {
		const auto& shaderMap = UniformBufferWriteDescriptorCache.at(&ubo_set);
		if (shaderMap.find(shaderHash) != shaderMap.end()) {
			const auto& writeDescriptors = shaderMap.at(shaderHash);
			return writeDescriptors;
		}
	}

	uint32_t framesInFlight = Renderer::GetConfig().FramesInFlight;
	Ref<VulkanShader> vulkanShader = vulkanMaterial->GetShader().As<VulkanShader>();
	if (vulkanShader->HasDescriptorSet(0)) {
		const auto& shaderDescriptorSets = vulkanShader->GetShaderDescriptorSets();
		if (!shaderDescriptorSets.empty()) {
			for (auto&& [binding, shaderUB] : shaderDescriptorSets[0].UniformBuffers) {
				auto& writeDescriptors = UniformBufferWriteDescriptorCache[&ubo_set][shaderHash];
				writeDescriptors.resize(framesInFlight);
				for (uint32_t frame = 0; frame < framesInFlight; frame++) {
					Ref<VulkanUniformBuffer> uniformBuffer = uniformBufferSet->Get(binding, 0, frame); // set = 0 for now

					VkWriteDescriptorSet writeDescriptorSet = {};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					writeDescriptorSet.pBufferInfo = &uniformBuffer->GetDescriptorBufferInfo();
					writeDescriptorSet.dstBinding = uniformBuffer->GetBinding();
					writeDescriptors[frame].push_back(writeDescriptorSet);
				}
			}
		}
	}

	return UniformBufferWriteDescriptorCache[&ubo_set][shaderHash];
}

static const std::vector<std::vector<VkWriteDescriptorSet>>& RT_RetrieveOrCreateStorageBufferWriteDescriptors(
	Ref<StorageBufferSet> storageBufferSet, Ref<VulkanMaterial> vulkanMaterial)
{
	std::unordered_map<StorageBufferSet*, std::unordered_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>>
		StorageBufferWriteDescriptorCache;

	size_t shaderHash = vulkanMaterial->GetShader()->GetHash();
	if (s_Data->StorageBufferWriteDescriptorCache.find(storageBufferSet.Raw()) != s_Data->StorageBufferWriteDescriptorCache.end()) {
		const auto& shaderMap = s_Data->StorageBufferWriteDescriptorCache.at(storageBufferSet.Raw());
		if (shaderMap.find(shaderHash) != shaderMap.end()) {
			const auto& writeDescriptors = shaderMap.at(shaderHash);
			return writeDescriptors;
		}
	}

	uint32_t framesInFlight = Renderer::GetConfig().FramesInFlight;
	Ref<VulkanShader> vulkanShader = vulkanMaterial->GetShader().As<VulkanShader>();
	if (vulkanShader->HasDescriptorSet(0)) {
		const auto& shaderDescriptorSets = vulkanShader->GetShaderDescriptorSets();
		if (!shaderDescriptorSets.empty()) {
			for (auto&& [binding, shaderSB] : shaderDescriptorSets[0].StorageBuffers) {
				auto& writeDescriptors = s_Data->StorageBufferWriteDescriptorCache[storageBufferSet.Raw()][shaderHash];
				writeDescriptors.resize(framesInFlight);
				for (uint32_t frame = 0; frame < framesInFlight; frame++) {
					Ref<VulkanStorageBuffer> storageBuffer = storageBufferSet->Get(binding, 0, frame); // set = 0 for now

					VkWriteDescriptorSet writeDescriptorSet = {};
					writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeDescriptorSet.descriptorCount = 1;
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					writeDescriptorSet.pBufferInfo = &storageBuffer->GetDescriptorBufferInfo();
					writeDescriptorSet.dstBinding = storageBuffer->GetBinding();
					writeDescriptors[frame].push_back(writeDescriptorSet);
				}
			}
		}
	}

	return s_Data->StorageBufferWriteDescriptorCache[storageBufferSet.Raw()][shaderHash];
}
*/

void Renderer::update_material_for_rendering(
	Ref<Vulkan::Material> material, const BufferSet<Disarray::UniformBuffer>* ubo_buffer, const BufferSet<Disarray::StorageBuffer>* sbo_buffer)
{
	if (ubo_buffer) {
		/*auto writeDescriptors = RT_RetrieveOrCreateUniformBufferWriteDescriptors(ubo_buffer, material);
		if (sbo_buffer) {
			const auto& storageBufferWriteDescriptors = RT_RetrieveOrCreateStorageBufferWriteDescriptors(sbo_buffer, material);

			const uint32_t framesInFlight = 3;
			for (uint32_t frame = 0; frame < framesInFlight; frame++) {
				writeDescriptors[frame].reserve(writeDescriptors[frame].size() + storageBufferWriteDescriptors[frame].size());
				writeDescriptors[frame].insert(
					writeDescriptors[frame].end(), storageBufferWriteDescriptors[frame].begin(), storageBufferWriteDescriptors[frame].end());
			}
		}
		*/

		// material->RT_UpdateForRendering(writeDescriptors);
	} else {
		Log::info("Material Update", "Updating material");
		// material->RT_UpdateForRendering();
	}
}

struct VkDescriptorSetHash {
	auto operator()(const VkDescriptorSet& descriptor_set) const -> std::size_t { return std::hash<VkDescriptorSet> {}(descriptor_set); }
};

struct VkDescriptorSetsSpanHash {
	auto operator()(const std::span<const VkDescriptorSet>& sets) const -> std::size_t
	{
		std::size_t hash_value = 0;

		for (const auto& descriptor_set : sets) {
			hash_value ^= VkDescriptorSetHash {}(descriptor_set) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
		}

		return hash_value;
	}
};

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
	const std::array desc {
		get_graphics_resource().get_descriptor_set(DescriptorSet(0)),
	};
	const auto span = std::span { desc.data(), desc.size() };
	bind_descriptor_sets(executor, pipeline, span);
}

namespace {
	auto internal_bind_buffer_set(IGraphicsResource& resource, Disarray::BufferSet<Disarray::UniformBuffer>* uniform_buffer_set,
		Disarray::BufferSet<Disarray::StorageBuffer>* storage_buffer_set)
	{
		std::vector<VkWriteDescriptorSet> write_descriptor_sets;
		uniform_buffer_set->for_each_frame_and_set(DescriptorSet(0),
			[&writes = write_descriptor_sets, image_count = resource.get_image_count(), set = resource.get_descriptor_set(DescriptorSet(0)), i = 0](
				const auto, const auto& buffer) mutable {
				const auto& vk_buffer = cast_to<Vulkan::UniformBuffer>(*buffer);

				auto& write_descriptor = writes.emplace_back();
				write_descriptor.descriptorCount = 1;
				write_descriptor.descriptorType = Vulkan::UniformBuffer::get_descriptor_type();
				write_descriptor.pBufferInfo = &vk_buffer.get_buffer_info();
				write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write_descriptor.dstSet = set;
				write_descriptor.dstBinding = i++;
			});

		if (storage_buffer_set != nullptr) {
			storage_buffer_set->for_each_frame_and_set(DescriptorSet(0),
				[&writes = write_descriptor_sets, image_count = resource.get_image_count(), set = resource.get_descriptor_set(DescriptorSet(0)),
					i = 7](const auto, const auto& buffer) mutable {
					const auto& vk_buffer = cast_to<Vulkan::StorageBuffer>(*buffer);

					auto& write_descriptor = writes.emplace_back();
					write_descriptor.descriptorCount = 1;
					write_descriptor.descriptorType = Vulkan::StorageBuffer::get_descriptor_type();
					write_descriptor.pBufferInfo = &vk_buffer.get_descriptor_info();
					write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write_descriptor.dstSet = set;
					write_descriptor.dstBinding = i++;
				});
		}

		return write_descriptor_sets;
	}
} // namespace

void Renderer::bind_buffer_set(Disarray::BufferSet<Disarray::UniformBuffer>& uniform_buffer_set)
{
	auto write_descriptor_sets = internal_bind_buffer_set(get_graphics_resource(), &uniform_buffer_set, nullptr);
	vkUpdateDescriptorSets(
		supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}

void Renderer::bind_buffer_set(
	Disarray::BufferSet<Disarray::UniformBuffer>& uniform_buffer_set, Disarray::BufferSet<Disarray::StorageBuffer>& storage_buffer_set)
{
	auto write_descriptor_sets = internal_bind_buffer_set(get_graphics_resource(), &uniform_buffer_set, &storage_buffer_set);
	vkUpdateDescriptorSets(
		supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
}

void Renderer::push_constant(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, const void* data, std::size_t size)
{
	get_graphics_resource().push_constant(executor, pipeline, data, size);
}

void Renderer::push_constant(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline)
{
	get_graphics_resource().push_constant(executor, pipeline);
}

void Renderer::bind_descriptor_sets(
	Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, const std::span<const VkDescriptorSet>& span)
{
	auto* pipeline_layout = cast_to<Vulkan::Pipeline>(pipeline).get_layout();
	vkCmdBindDescriptorSets(supply_cast<Vulkan::CommandExecutor>(executor), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0,
		static_cast<std::uint32_t>(span.size()), span.data(), 0, nullptr);

	/*

	VkDescriptorSetsSpanHash hasher;
	const auto calculated = hasher(span);
	if (calculated != bound_descriptor_set_hash) {
		bound_descriptor_set_hash = calculated;
		return;
	}
	*/
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const Disarray::Material& material, const TransformMatrix& transform, const ColourVector& colour)
{
	draw_mesh(executor, mesh.get_vertices(), mesh.get_indices(), mesh_pipeline, material, transform, colour);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline,
	const TransformMatrix& transform, const ColourVector& colour)
{
	draw_mesh(executor, mesh.get_vertices(), mesh.get_indices(), mesh_pipeline, transform, colour);
}

void Renderer::draw_mesh_without_bind(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh)
{
	if (mesh.invalid()) {
		return;
	}

	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

	std::array<VkBuffer, 1> arr {};
	arr[0] = supply_cast<Vulkan::VertexBuffer>(mesh.get_vertices());
	std::array<VkDeviceSize, 1> offsets = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());
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
	bind_pipeline(executor, pipeline, PipelineBindPoint::BindPointGraphics);
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

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices,
	const Disarray::Pipeline& mesh_pipeline, const Disarray::Material&, const TransformMatrix& transform, const ColourVector& colour)
{
	draw_mesh(executor, vertices, indices, mesh_pipeline, transform, colour);
}

void Renderer::draw_mesh(Disarray::CommandExecutor& executor, const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices,
	const Disarray::Pipeline& mesh_pipeline, const TransformMatrix& transform, const ColourVector& colour)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	const auto& pipeline = cast_to<Vulkan::Pipeline>(mesh_pipeline);
	bind_pipeline(executor, pipeline, PipelineBindPoint::BindPointGraphics);

	auto& push_constant = get_graphics_resource().get_editable_push_constant();

	push_constant.object_transform = transform;
	push_constant.colour = colour;
	vkCmdPushConstants(
		command_buffer, pipeline.get_layout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant), &push_constant);

	bind_descriptor_sets(executor, pipeline);

	const std::array arr { supply_cast<Vulkan::VertexBuffer>(vertices) };
	const std::array offsets = { VkDeviceSize { 0 } };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, arr.data(), offsets.data());

	if (pipeline.get_properties().polygon_mode == PolygonMode::Line) {
		vkCmdSetLineWidth(command_buffer, pipeline.get_properties().line_width);
	}

	const auto& vk_indices = cast_to<Vulkan::IndexBuffer>(indices);
	vkCmdBindIndexBuffer(command_buffer, vk_indices.supply(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(command_buffer, static_cast<std::uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Renderer::text_rendering_pass(Disarray::CommandExecutor& executor) { text_renderer.render(*this, executor); }

void Renderer::planar_geometry_pass(Disarray::CommandExecutor& executor) { batch_renderer.submit(*this, executor); }

void Renderer::fullscreen_quad_pass(Disarray::CommandExecutor& executor, const Disarray::Pipeline& fullscreen_pipeline)
{
	const auto& framebuffer = fullscreen_pipeline.get_framebuffer();
	begin_pass(executor, framebuffer, false, RenderAreaExtent { framebuffer });

	auto* cmd = supply_cast<Vulkan::CommandExecutor>(executor);
	bind_pipeline(executor, fullscreen_pipeline, PipelineBindPoint::BindPointGraphics);
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

void Renderer::begin_pass(
	Disarray::CommandExecutor& executor, const Disarray::Framebuffer& framebuffer, bool explicit_clear, const RenderAreaExtent& render_area_extent)
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

void Renderer::bind_buffer(Disarray::CommandExecutor& executor, const Disarray::VertexBuffer& buffer)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	auto offsets = std::array<VkDeviceSize, 1> { 0 };
	auto vk_buffer = supply_cast<Vulkan::VertexBuffer>(buffer);
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_buffer, offsets.data());
}

void Renderer::bind_buffer(Disarray::CommandExecutor& executor, const Disarray::IndexBuffer& buffer)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	auto offsets = std::array<VkDeviceSize, 1> { 0 };
	auto vk_buffer = supply_cast<Vulkan::IndexBuffer>(buffer);
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_buffer, offsets.data());
}

void Renderer::draw_static_mesh(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline,
	const BufferSet<Disarray::UniformBuffer>& uniform_buffer_set, const BufferSet<Disarray::StorageBuffer>& storage_buffer_set,
	const StaticSubmesh& submesh, const Disarray::MaterialTable& material_table, const TransformMatrix& transform)
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);

	/*Ref<MeshSource> meshSource = mesh->GetMeshSource();
	Ref<VulkanVertexBuffer> vulkanMeshVB = meshSource->GetVertexBuffer().As<VulkanVertexBuffer>();
	VkBuffer vbMeshBuffer = vulkanMeshVB->GetVulkanBuffer();
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &vbMeshBuffer, offsets);

	auto vulkanMeshIB = Ref<VulkanIndexBuffer>(meshSource->GetIndexBuffer());
	VkBuffer ibBuffer = vulkanMeshIB->GetVulkanBuffer();
	vkCmdBindIndexBuffer(command_buffer, ibBuffer, 0, VK_INDEX_TYPE_UINT32);*/

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, supply_cast<Vulkan::Pipeline>(pipeline));

	auto vk_material = material_table.get_material(submesh.MaterialIndex)->get_material().as<Vulkan::Material>();
	update_material_for_rendering(vk_material, &uniform_buffer_set, &storage_buffer_set);

	// NOTE: Descriptor Set 0 is owned by the renderer
	std::array<VkDescriptorSet, 2> descriptorSets {};

	auto* layout = cast_to<Vulkan::Pipeline>(pipeline).get_layout();
	vkCmdBindDescriptorSets(
		command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, nullptr);

	// auto uniformStorageBuffer = vk_material->GetUniformStorageBuffer();
	// vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, uniformStorageBuffer.Size, uniformStorageBuffer.Data);

	// add instance count!
	vkCmdDrawIndexed(command_buffer, submesh.IndexCount, 1, submesh.BaseIndex, submesh.BaseVertex, 0);
}

} // namespace Disarray::Vulkan
