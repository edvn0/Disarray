#include "DisarrayPCH.hpp"

#include "vulkan/GraphicsResource.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#include <magic_enum.hpp>

#include <array>

#include "core/Collections.hpp"
#include "core/Types.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "magic_enum_switch.hpp"
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
		current_frame_ubos[3] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(ShadowPassUBO),
			});
		current_frame_ubos[4] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(DirectionalLightUBO),
			});
		current_frame_ubos[5] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(GlyphUBO),
			});
		frame_index_ubo_map.try_emplace(i, std::move(current_frame_ubos));
	}

	initialise_descriptors();
}

void GraphicsResource::recreate(bool should_clean, const Extent& extent) { initialise_descriptors(should_clean); }

static auto create_set_zero_bindings()
{
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

	auto shadow_pass_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	shadow_pass_binding.descriptorCount = 1;
	shadow_pass_binding.binding = 3;
	shadow_pass_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	shadow_pass_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	auto directional_light_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	directional_light_binding.descriptorCount = 1;
	directional_light_binding.binding = 4;
	directional_light_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	directional_light_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	auto glyph_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	glyph_binding.descriptorCount = 1;
	glyph_binding.binding = 5;
	glyph_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	glyph_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	return std::array {
		default_binding,
		camera_binding,
		point_light_binding,
		shadow_pass_binding,
		directional_light_binding,
		glyph_binding,
	};
}

static auto create_set_one_bindings()
{
	// Default framebuffer
	auto image_binding_0 = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_binding_0.descriptorCount = 1;
	image_binding_0.binding = 0;
	image_binding_0.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image_binding_0.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Depth texture
	auto image_binding_1 = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_binding_1.descriptorCount = 1;
	image_binding_1.binding = 1;
	image_binding_1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image_binding_1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Font texture
	auto image_binding_2 = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_binding_2.descriptorCount = 1;
	image_binding_2.binding = 2;
	image_binding_2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image_binding_2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	return std::array {
		image_binding_0,
		image_binding_1,
		image_binding_2,
	};
}

static auto create_set_two_bindings()
{
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

	auto glyph_texture_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	glyph_texture_binding.descriptorCount = 128;
	glyph_texture_binding.binding = 2;
	glyph_texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	glyph_texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	auto glyph_array_sampler_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	glyph_array_sampler_binding.descriptorCount = 1;
	glyph_array_sampler_binding.binding = 3;
	glyph_array_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	glyph_array_sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	return std::array { image_array_binding, image_array_sampler_binding, glyph_texture_binding, glyph_array_sampler_binding };
}

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

	auto set_zero_bindings = create_set_zero_bindings();
	auto set_one_bindings = create_set_one_bindings();
	auto set_two_bindings = create_set_two_bindings();

	auto layout_create_info = vk_structures<VkDescriptorSetLayoutCreateInfo> {}();
	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_zero_bindings.size());
	layout_create_info.pBindings = set_zero_bindings.data();

	VkDescriptorSetLayout set_zero_ubos_layout = nullptr;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &set_zero_ubos_layout));

	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_one_bindings.size());
	layout_create_info.pBindings = set_one_bindings.data();
	VkDescriptorSetLayout set_one_images_layout = nullptr;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &set_one_images_layout));

	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_two_bindings.size());
	layout_create_info.pBindings = set_two_bindings.data();
	VkDescriptorSetLayout set_two_image_array_layout = nullptr;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &set_two_image_array_layout));

	layouts = { set_zero_ubos_layout, set_one_images_layout, set_two_image_array_layout };
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

	std::vector<VkDescriptorSetLayout> desc_layouts(layouts.size());
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
		desc_sets.resize(layouts.size());
		vkAllocateDescriptorSets(vk_device, &alloc_info, desc_sets.data());
		descriptor_sets.try_emplace(i, std::move(desc_sets));
	}

	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		const auto& ubos = frame_index_ubo_map.at(i);
		auto& frame_descriptor_sets = descriptor_sets.at(i);

		VkDescriptorBufferInfo renderer_buffer {};
		renderer_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos.at(0));
		renderer_buffer.offset = 0;
		renderer_buffer.range = sizeof(UBO);

		VkDescriptorBufferInfo camera_buffer {};
		camera_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos.at(1));
		camera_buffer.offset = 0;
		camera_buffer.range = sizeof(CameraUBO);

		VkDescriptorBufferInfo point_light_buffer {};
		point_light_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos.at(2));
		point_light_buffer.offset = 0;
		point_light_buffer.range = sizeof(PointLights);

		VkDescriptorBufferInfo shadow_pass_buffer {};
		shadow_pass_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos.at(3));
		shadow_pass_buffer.offset = 0;
		shadow_pass_buffer.range = sizeof(ShadowPassUBO);

		VkDescriptorBufferInfo directional_light_buffer {};
		directional_light_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos.at(4));
		directional_light_buffer.offset = 0;
		directional_light_buffer.range = sizeof(DirectionalLightUBO);

		VkDescriptorBufferInfo glyph_ubo_buffer {};
		glyph_ubo_buffer.buffer = supply_cast<Vulkan::UniformBuffer>(*ubos.at(5));
		glyph_ubo_buffer.offset = 0;
		glyph_ubo_buffer.range = sizeof(GlyphUBO);

		auto write_sets = vk_structures<VkWriteDescriptorSet, 6> {}.multiple();
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

		write_sets[3].dstSet = frame_descriptor_sets.at(0);
		write_sets[3].dstBinding = 3;
		write_sets[3].dstArrayElement = 0;
		write_sets[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[3].descriptorCount = 1;
		write_sets[3].pBufferInfo = &shadow_pass_buffer;

		write_sets[4].dstSet = frame_descriptor_sets.at(0);
		write_sets[4].dstBinding = 4;
		write_sets[4].dstArrayElement = 0;
		write_sets[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[4].descriptorCount = 1;
		write_sets[4].pBufferInfo = &directional_light_buffer;

		write_sets[5].dstSet = frame_descriptor_sets.at(0);
		write_sets[5].dstBinding = 5;
		write_sets[5].dstArrayElement = 0;
		write_sets[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[5].descriptorCount = 1;
		write_sets[5].pBufferInfo = &glyph_ubo_buffer;

		vkUpdateDescriptorSets(vk_device, static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
	}
}

void GraphicsResource::expose_to_shaders(const Disarray::Image& image, DescriptorSet descriptor_set, DescriptorBinding binding)
{
	auto write_sets = descriptor_write_sets_per_frame(descriptor_set);
	const auto& info = cast_to<Vulkan::Image>(image).get_descriptor_info();

	Log::info("GraphicsResource", "Exposing '{}' to shader at binding {}", image.get_properties().debug_name, binding);
	for (auto& write_set : write_sets) {
		write_set.dstBinding = binding;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_set.descriptorCount = 1;
		write_set.pImageInfo = &info;
	}

	vkUpdateDescriptorSets(supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
}

void GraphicsResource::expose_to_shaders(std::span<const Ref<Disarray::Texture>> span, DescriptorSet set, DescriptorBinding binding)
{
	ensure(binding == 0, "Safety before we can generalise!");
	auto image_infos = Collections::map(span, [](const Ref<Disarray::Texture>& texture) -> VkDescriptorImageInfo {
		const auto& desc_info = cast_to<Vulkan::Image>(texture->get_image()).get_descriptor_info();
		return {
			.sampler = nullptr,
			.imageView = desc_info.imageView,
			.imageLayout = desc_info.imageLayout,
		};
	});

	internal_expose_to_shaders(cast_to<Vulkan::Image>(span[0]->get_image()).get_descriptor_info().sampler, image_infos, set, binding);
}

void GraphicsResource::expose_to_shaders(std::span<const Disarray::Texture*> span, DescriptorSet set, DescriptorBinding binding)
{
	auto image_infos = Collections::map(span, [](const Disarray::Texture* texture) -> VkDescriptorImageInfo {
		const auto& desc_info = cast_to<Vulkan::Image>(texture->get_image()).get_descriptor_info();
		return {
			.sampler = nullptr,
			.imageView = desc_info.imageView,
			.imageLayout = desc_info.imageLayout,
		};
	});

	internal_expose_to_shaders(cast_to<Vulkan::Image>(span[0]->get_image()).get_descriptor_info().sampler, image_infos, set, binding);
}

void GraphicsResource::update_ubo()
{
	auto& current_uniform = frame_index_ubo_map.at(swapchain.get_current_frame());
	current_uniform.at(0)->set_data(&uniform, sizeof(UBO));
	current_uniform.at(1)->set_data(&camera_ubo, sizeof(CameraUBO));
	current_uniform.at(2)->set_data(&lights, sizeof(PointLights));
	current_uniform.at(3)->set_data(&shadow_pass_ubo, sizeof(ShadowPassUBO));
	current_uniform.at(4)->set_data(&directional_light_ubo, sizeof(DirectionalLightUBO));
	current_uniform.at(5)->set_data(&glyph_ubo, sizeof(GlyphUBO));
}

void GraphicsResource::update_ubo(UBOIdentifier identifier)
{
	magic_enum::enum_switch(
		[this](auto val) {
			constexpr UBOIdentifier identifier = val;
			update_ubo(static_cast<std::size_t>(identifier));
		},
		identifier);
}

void GraphicsResource::update_ubo(std::size_t index)
{
	auto& current_uniform = frame_index_ubo_map.at(swapchain.get_current_frame());
	switch (index) {
	case 0: {
		current_uniform.at(0)->set_data(&uniform, sizeof(UBO));
		return;
	}
	case 1: {
		current_uniform.at(1)->set_data(&camera_ubo, sizeof(CameraUBO));
		return;
	}
	case 2: {
		current_uniform.at(2)->set_data(&lights, sizeof(PointLights));
		return;
	}
	case 3: {
		current_uniform.at(3)->set_data(&shadow_pass_ubo, sizeof(ShadowPassUBO));
		return;
	}
	case 4: {
		current_uniform.at(4)->set_data(&directional_light_ubo, sizeof(DirectionalLightUBO));
		return;
	}
	case 5: {
		current_uniform.at(5)->set_data(&glyph_ubo, sizeof(GlyphUBO));
		return;
	}
	}
}

void GraphicsResource::cleanup_graphics_resource()
{
	const auto& vk_device = supply_cast<Vulkan::Device>(device);
	descriptor_sets.clear();
	Collections::for_each(layouts, [&vk_device](VkDescriptorSetLayout& layout) { vkDestroyDescriptorSetLayout(vk_device, layout, nullptr); });
	vkDestroyDescriptorPool(vk_device, pool, nullptr);
}

GraphicsResource::~GraphicsResource() { cleanup_graphics_resource(); }

auto GraphicsResource::descriptor_write_sets_per_frame(std::size_t descriptor_set) -> std::vector<VkWriteDescriptorSet>
{
	std::vector<VkWriteDescriptorSet> output {};
	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		auto& write_set = output.emplace_back();
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_sets.at(i).at(descriptor_set);
	}
	return output;
}

void GraphicsResource::internal_expose_to_shaders(
	VkSampler input_sampler, const std::vector<VkDescriptorImageInfo>& image_infos, DescriptorSet set, DescriptorBinding binding)
{
	const auto* data = image_infos.data();
	const auto size = static_cast<std::uint32_t>(image_infos.size());

	std::vector<VkWriteDescriptorSet> write_sets {};
	auto* default_sampler = input_sampler;
	auto sampler_info = VkDescriptorImageInfo {
		.sampler = default_sampler,
		.imageView = nullptr,
		.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		auto& write_set = write_sets.emplace_back();
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_sets.at(i).at(set);
		write_set.dstBinding = binding;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write_set.descriptorCount = size;
		write_set.pImageInfo = data;

		auto& sampler = write_sets.emplace_back();
		sampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		sampler.dstSet = descriptor_sets.at(i).at(set);
		sampler.dstBinding = binding + 1;
		sampler.dstArrayElement = 0;
		sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		sampler.descriptorCount = 1;
		sampler.pImageInfo = &sampler_info;
	}

	vkUpdateDescriptorSets(supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
}

} // namespace Disarray::Vulkan
