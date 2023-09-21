#include "DisarrayPCH.hpp"

#include "vulkan/GraphicsResource.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <array>

#include "core/Collections.hpp"
#include "core/Types.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/RendererProperties.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/UniformBuffer.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray::Vulkan {

GraphicsResource::GraphicsResource(const Disarray::Device& dev, const Disarray::Swapchain& sc)
	: device(dev)
	, swapchain(sc)
	, swapchain_image_count(swapchain.image_count())
	, pipeline_cache(dev, FS::shader_directory())
	, texture_cache(dev, FS::texture_directory())
{
	frame_index_ubo_map.reserve(swapchain_image_count);
	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		UBOArray current_frame_ubos {};
		current_frame_ubos[0] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(UBO),
			});
		current_frame_ubos[1] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(CameraUBO),
			});
		current_frame_ubos[2] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(PointLights),
			});
		frame_index_ubo_map.try_emplace(i, std::move(current_frame_ubos));
	}

	initialise_descriptors();
}

void GraphicsResource::recreate(bool should_clean, const Extent& extent) { initialise_descriptors(should_clean); }

void GraphicsResource::initialise_descriptors(bool should_clean)
{
	if (should_clean) {
		cleanup_graphics_resource();
	}

	auto* vk_device = supply_cast<Vulkan::Device>(device);

	TextureCacheCreationProperties texture_properties {
		.key = "viking",
		.debug_name = "viking",
		.path = FS::texture("viking_room.png"),
		.format = ImageFormat::SBGR,
	};
	const auto& viking_room = texture_cache.put(texture_properties);

	auto default_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	default_binding.descriptorCount = 1;
	default_binding.binding = 0;
	default_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	default_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	auto camera_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	camera_binding.descriptorCount = 1;
	camera_binding.binding = 1;
	camera_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	camera_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	auto point_light_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	point_light_binding.descriptorCount = 1;
	point_light_binding.binding = 2;
	point_light_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	point_light_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	auto image_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_binding.descriptorCount = 1;
	image_binding.binding = 0;
	image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	auto image_array_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_array_binding.descriptorCount = max_allowed_texture_indices;
	image_array_binding.binding = 0;
	image_array_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	image_array_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	auto image_array_sampler_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_array_sampler_binding.descriptorCount = 1;
	image_array_sampler_binding.binding = 1;
	image_array_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	image_array_sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	auto layout_create_info = vk_structures<VkDescriptorSetLayoutCreateInfo> {}();
	std::array set_zero_bindings = { default_binding, camera_binding, point_light_binding };
	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_zero_bindings.size());
	layout_create_info.pBindings = set_zero_bindings.data();

	VkDescriptorSetLayout set_zero_layout = nullptr;
	VkDescriptorSetLayout image_layout = nullptr;
	VkDescriptorSetLayout image_array_layout = nullptr;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &set_zero_layout));

	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = &image_binding;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &image_layout));

	std::array set_two_bindings = { image_array_binding, image_array_sampler_binding };
	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_two_bindings.size());
	layout_create_info.pBindings = set_two_bindings.data();
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &image_array_layout));

	layouts = { set_zero_layout, image_layout, image_array_layout };
	static constexpr auto descriptor_pool_max_size = 1000;

	const std::array<VkDescriptorPoolSize, 12> sizes = [](auto size) {
		std::array<VkDescriptorPoolSize, 12> temp {};
		temp.at(0) = { VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_pool_max_size };
		temp.at(1) = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_pool_max_size };
		temp.at(2) = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_pool_max_size };
		temp.at(3) = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_pool_max_size };
		temp.at(4) = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_pool_max_size };
		temp.at(5) = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_pool_max_size };
		temp.at(6) = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_pool_max_size };
		temp.at(7) = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_pool_max_size };
		temp.at(8) = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_pool_max_size };
		temp.at(9) = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_pool_max_size };
		temp.at(10) = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptor_pool_max_size };
		temp.at(11) = { VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, descriptor_pool_max_size };
		return temp;
	}(descriptor_pool_max_size);

	auto pool_create_info = vk_structures<VkDescriptorPoolCreateInfo> {}();
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.poolSizeCount = static_cast<std::uint32_t>(sizes.size());
	pool_create_info.pPoolSizes = sizes.data();
	pool_create_info.maxSets = descriptor_pool_max_size * static_cast<std::uint32_t>(sizes.size());

	verify(vkCreateDescriptorPool(vk_device, &pool_create_info, nullptr, &pool));

	std::vector<VkDescriptorSetLayout> desc_layouts(set_count);
	for (std::size_t i = 0; i < desc_layouts.size(); i++) {
		desc_layouts[i] = layouts[i];
	}

	VkDescriptorSetAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = pool;
	alloc_info.descriptorSetCount = static_cast<std::uint32_t>(desc_layouts.size());
	alloc_info.pSetLayouts = desc_layouts.data();

	for (auto i = 0U; i < swapchain_image_count; i++) {
		std::vector<VkDescriptorSet> desc_sets {};
		desc_sets.resize(set_count);
		vkAllocateDescriptorSets(vk_device, &alloc_info, desc_sets.data());
		descriptor_sets.try_emplace(i, std::move(desc_sets));
	}

	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		const auto& ubos = frame_index_ubo_map.at(i);
		auto& frame_descriptor_sets = descriptor_sets.at(i);

		VkDescriptorBufferInfo renderer_buffer {};
		renderer_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos[0]);
		renderer_buffer.offset = 0;
		renderer_buffer.range = sizeof(UBO);

		VkDescriptorBufferInfo camera_buffer {};
		camera_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos[1]);
		camera_buffer.offset = 0;
		camera_buffer.range = sizeof(CameraUBO);

		VkDescriptorBufferInfo point_light_buffer {};
		point_light_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos[2]);
		point_light_buffer.offset = 0;
		point_light_buffer.range = sizeof(PointLights);

		auto write_sets = vk_structures<VkWriteDescriptorSet, 4> {}.multiple();
		write_sets[0].dstSet = frame_descriptor_sets.at(0);
		write_sets[0].dstBinding = 0;
		write_sets[0].dstArrayElement = 0;
		write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[0].descriptorCount = 1;
		write_sets[0].pBufferInfo = &renderer_buffer;

		write_sets[1].dstSet = frame_descriptor_sets.at(0);
		write_sets[1].dstBinding = 1;
		write_sets[1].dstArrayElement = 0;
		write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[1].descriptorCount = 1;
		write_sets[1].pBufferInfo = &camera_buffer;

		write_sets[2].dstSet = frame_descriptor_sets.at(0);
		write_sets[2].dstBinding = 2;
		write_sets[2].dstArrayElement = 0;
		write_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[2].descriptorCount = 1;
		write_sets[2].pBufferInfo = &point_light_buffer;

		VkDescriptorImageInfo image_info = cast_to<Vulkan::Image>(viking_room->get_image()).get_descriptor_info();
		write_sets[3].dstSet = frame_descriptor_sets.at(1);
		write_sets[3].dstBinding = 0;
		write_sets[3].dstArrayElement = 0;
		write_sets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_sets[3].descriptorCount = 1;
		write_sets[3].pImageInfo = &image_info;

		vkUpdateDescriptorSets(vk_device, static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
	}
}

void GraphicsResource::expose_to_shaders(const Disarray::Image& image)
{
	// TODO!
}

void GraphicsResource::expose_to_shaders(std::span<const Ref<Disarray::Texture>> span)
{
	auto image_infos = Collections::map(span, [](const Ref<Disarray::Texture>& texture) -> VkDescriptorImageInfo {
		const auto& desc_info = cast_to<Vulkan::Image>(texture->get_image()).get_descriptor_info();
		return {
			.sampler = nullptr,
			.imageView = desc_info.imageView,
			.imageLayout = desc_info.imageLayout,
		};
	});

	const auto* data = image_infos.data();
	const auto size = static_cast<std::uint32_t>(image_infos.size());

	std::vector<VkWriteDescriptorSet> write_sets {};
	static VkSampler default_sampler = cast_to<Vulkan::Image>(span[0]->get_image()).get_descriptor_info().sampler;
	static VkDescriptorImageInfo sampler_info {
		.sampler = default_sampler,
		.imageView = nullptr,
		.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		auto& write_set = write_sets.emplace_back();
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_sets.at(i).at(2);
		write_set.dstBinding = 0;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write_set.descriptorCount = size;
		write_set.pImageInfo = data;

		auto& sampler = write_sets.emplace_back();
		sampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		sampler.dstSet = descriptor_sets.at(i).at(2);
		sampler.dstBinding = 1;
		sampler.dstArrayElement = 0;
		sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		sampler.descriptorCount = 1;
		sampler.pImageInfo = &sampler_info;
	}

	vkUpdateDescriptorSets(supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
}

void GraphicsResource::update_ubo()
{
	auto& current_uniform = frame_index_ubo_map.at(swapchain.get_current_frame());
	current_uniform.at(0)->set_data(&uniform, sizeof(UBO));
	current_uniform.at(1)->set_data(&camera_ubo, sizeof(CameraUBO));
	current_uniform.at(2)->set_data(&lights, sizeof(PointLights));
}

void GraphicsResource::cleanup_graphics_resource()
{
	const auto& vk_device = supply_cast<Vulkan::Device>(device);
	descriptor_sets.clear();
	Collections::for_each(layouts, [&vk_device](VkDescriptorSetLayout& layout) { vkDestroyDescriptorSetLayout(vk_device, layout, nullptr); });
	vkDestroyDescriptorPool(vk_device, pool, nullptr);
}

GraphicsResource::~GraphicsResource() { cleanup_graphics_resource(); }

} // namespace Disarray::Vulkan
