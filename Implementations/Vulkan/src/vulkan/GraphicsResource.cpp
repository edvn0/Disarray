#include "DisarrayPCH.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <magic_enum.hpp>
#include <magic_enum_switch.hpp>

#include <array>

#include "core/Collections.hpp"
#include "core/Types.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/Image.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/GraphicsResource.hpp"
#include "vulkan/IndexBuffer.hpp"
#include "vulkan/Mesh.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/StorageBuffer.hpp"
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
	for (auto i = FrameIndex { 0 }; i < swapchain_image_count; i++) {
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

namespace {
	auto create_set_zero_bindings()
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

	auto create_set_one_bindings()
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

	auto create_set_two_bindings()
	{
		auto glyph_texture_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		glyph_texture_binding.descriptorCount = 128;
		glyph_texture_binding.binding = 0;
		glyph_texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		glyph_texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		auto glyph_array_sampler_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
		glyph_array_sampler_binding.descriptorCount = 1;
		glyph_array_sampler_binding.binding = 1;
		glyph_array_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		glyph_array_sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		auto skybox_image = vk_structures<VkDescriptorSetLayoutBinding> {}();
		skybox_image.descriptorCount = 1;
		skybox_image.binding = 2;
		skybox_image.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		skybox_image.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		return std::array { glyph_texture_binding, glyph_array_sampler_binding, skybox_image };
	}

	template <std::size_t Count> auto create_set_three_bindings()
	{
		std::array<VkDescriptorSetLayoutBinding, Count> transform_data_ssbo_bindings {};
		std::uint32_t i = 0;
		for (auto& binding : transform_data_ssbo_bindings) {
			binding.descriptorCount = 1;
			binding.binding = i++;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			binding.pImmutableSamplers = nullptr;
		}

		return transform_data_ssbo_bindings;
	}
} // namespace

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
	texture_cache.put(texture_properties);

	auto set_zero_bindings = create_set_zero_bindings();
	auto set_one_bindings = create_set_one_bindings();
	auto set_two_bindings = create_set_two_bindings();
	auto set_three_bindings = create_set_three_bindings<4>();

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

	VkDescriptorSetLayout set_three_ssbo_layout = nullptr;
	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_three_bindings.size());
	layout_create_info.pBindings = set_three_bindings.data();
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &set_three_ssbo_layout));

	layouts = { set_zero_ubos_layout, set_one_images_layout, set_two_image_array_layout, set_three_ssbo_layout };
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

	for (auto i = FrameIndex { 0 }; i < swapchain_image_count; i++) {
		std::vector<VkDescriptorSet> desc_sets {};
		desc_sets.resize(layouts.size());
		vkAllocateDescriptorSets(vk_device, &alloc_info, desc_sets.data());
		descriptor_sets.try_emplace(i, std::move(desc_sets));
	}

	static constexpr auto get_buffer_info
		= [](const Scope<Vulkan::UniformBuffer>& buffer) { return &cast_to<Vulkan::UniformBuffer>(*buffer).get_buffer_info(); };

	for (auto i = FrameIndex { 0 }; i < swapchain_image_count; i++) {
		const auto& ubos = frame_index_ubo_map.at(i);
		auto& frame_descriptor_sets = descriptor_sets.at(i);

		auto write_sets = vk_structures<VkWriteDescriptorSet, std::tuple_size<UBOArray> {}> {}.multiple();
		for (auto write_set_index = 0ULL; write_set_index < write_sets.size(); write_set_index++) {
			auto& write_set = write_sets.at(write_set_index);
			write_set.dstSet = frame_descriptor_sets.at(0);
			write_set.dstBinding = write_set_index;
			write_set.descriptorCount = 1;
			write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_set.pBufferInfo = get_buffer_info(ubos.at(write_set_index));
		}

		vkUpdateDescriptorSets(vk_device, static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
	}
}

void GraphicsResource::expose_to_shaders(const Disarray::Image& image, DescriptorSet descriptor_set, DescriptorBinding binding)
{
	auto write_sets = descriptor_write_sets_per_frame(descriptor_set);
	const auto& info = cast_to<Vulkan::Image>(image).get_descriptor_info();

	Log::info("GraphicsResource", "Exposing '{}' to shader at set {} - binding {}", image.get_properties().debug_name, descriptor_set, binding);
	for (auto& write_set : write_sets) {
		write_set.dstBinding = binding.value;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_set.descriptorCount = 1;
		write_set.pImageInfo = &info;
	}

	vkUpdateDescriptorSets(supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
}

void GraphicsResource::expose_to_shaders(std::span<const Ref<Disarray::Texture>> textures, DescriptorSet set, DescriptorBinding binding)
{
	ensure(binding == 0, "Safety before we can generalise!");
	auto image_infos = Collections::map(textures, [](const Ref<Disarray::Texture>& texture) -> VkDescriptorImageInfo {
		const auto& desc_info = cast_to<Vulkan::Image>(texture->get_image()).get_descriptor_info();
		return {
			.sampler = nullptr,
			.imageView = desc_info.imageView,
			.imageLayout = desc_info.imageLayout,
		};
	});

	internal_expose_to_shaders(cast_to<Vulkan::Image>(textures[0]->get_image()).get_descriptor_info().sampler, image_infos, set, binding);
}

void GraphicsResource::expose_to_shaders(const Disarray::StorageBuffer& buffer, DescriptorSet set, DescriptorBinding binding)
{
	auto write_sets = descriptor_write_sets_per_frame(set);
	const auto& vk_buffer = cast_to<Vulkan::StorageBuffer>(buffer);

	for (auto& write_set : write_sets) {
		write_set.dstBinding = binding.value;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = Vulkan::StorageBuffer::get_descriptor_type();
		write_set.descriptorCount = 1;
		write_set.pBufferInfo = &vk_buffer.get_descriptor_info();
	}

	vkUpdateDescriptorSets(supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
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
	auto& current_uniform = frame_index_ubo_map.at(swapchain.get_current_frame_index());
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
	auto& current_uniform = frame_index_ubo_map.at(swapchain.get_current_frame_index());
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
	default:
		unreachable("Invalid UBO index");
	}
}

void GraphicsResource::cleanup_graphics_resource()
{
	const auto& vk_device = supply_cast<Vulkan::Device>(device);
	descriptor_sets.clear();
	Collections::for_each(layouts, [&vk_device](VkDescriptorSetLayout& layout) { vkDestroyDescriptorSetLayout(vk_device, layout, nullptr); });
	layouts = {};
	vkDestroyDescriptorPool(vk_device, pool, nullptr);
}

GraphicsResource::~GraphicsResource() { cleanup_graphics_resource(); }

auto GraphicsResource::descriptor_write_sets_per_frame(DescriptorSet descriptor_set) -> std::vector<VkWriteDescriptorSet>
{
	std::vector<VkWriteDescriptorSet> output {};
	for (auto i = FrameIndex { 0 }; i < swapchain_image_count; i++) {
		auto& write_set = output.emplace_back();
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_sets.at(i).at(descriptor_set.value);
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
	VkDescriptorImageInfo sampler_info {
		.sampler = default_sampler,
		.imageView = nullptr,
		.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	for (std::size_t i = 0; i < swapchain_image_count; i++) {
		FrameIndex index { i };
		auto& write_set = write_sets.emplace_back();
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = descriptor_sets.at(index).at(set.value);
		write_set.dstBinding = binding.value;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write_set.descriptorCount = size;
		write_set.pImageInfo = data;

		auto& sampler = write_sets.emplace_back();
		sampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		sampler.dstSet = descriptor_sets.at(index).at(set.value);
		sampler.dstBinding = (binding + 1).value;
		sampler.dstArrayElement = 0;
		sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		sampler.descriptorCount = 1;
		sampler.pImageInfo = &sampler_info;
	}

	vkUpdateDescriptorSets(supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
}

} // namespace Disarray::Vulkan
