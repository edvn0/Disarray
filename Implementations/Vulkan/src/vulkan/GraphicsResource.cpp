#include "DisarrayPCH.hpp"

#include "vulkan/GraphicsResource.hpp"

// clang-format off
#include "vulkan/Renderer.hpp"
// clang-format on

#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#include <array>

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

namespace Disarray::Vulkan {

GraphicsResource::GraphicsResource(const Disarray::Device& dev, const Disarray::Swapchain& sc)
	: device(dev)
	, swapchain(sc)
	, pipeline_cache(dev, "Assets/Shaders")
	, texture_cache(dev, "Assets/Textures")
{
	frame_ubos.resize(sc.image_count());
	for (std::size_t i = 0; i < sc.image_count(); i++) {
		auto& current_frame_ubos = frame_ubos[i];
		current_frame_ubos[0] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(UBO),
				.count = 1,
				.binding = 0,
			});
		current_frame_ubos[1] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(CameraUBO),
				.count = 1,
				.binding = 1,
			});
		current_frame_ubos[2] = make_scope<Vulkan::UniformBuffer>(device,
			BufferProperties {
				.size = sizeof(PointLights),
				.count = 1,
				.binding = 2,
			});
	}

	initialise_descriptors();
}

void GraphicsResource::initialise_descriptors()
{
	auto* vk_device = supply_cast<Vulkan::Device>(device);

	TextureCacheCreationProperties texture_properties { .key = "viking", .debug_name = "viking" };
	texture_properties.path = "Assets/Textures/viking_room.png";
	texture_properties.format = ImageFormat::SRGB;
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

	auto layout_create_info = vk_structures<VkDescriptorSetLayoutCreateInfo> {}();
	std::array<VkDescriptorSetLayoutBinding, 3> set_zero_bindings = { default_binding, camera_binding, point_light_binding };
	layout_create_info.bindingCount = static_cast<std::uint32_t>(set_zero_bindings.size());
	layout_create_info.pBindings = set_zero_bindings.data();

	VkDescriptorSetLayout set_zero_layout = nullptr;
	VkDescriptorSetLayout image_layout = nullptr;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &set_zero_layout));

	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = &image_binding;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &image_layout));

	layouts = { set_zero_layout, image_layout };
	static constexpr auto descriptor_pool_max_size = 1000;

	std::array<VkDescriptorPoolSize, 12> sizes {};
	sizes.at(0) = { VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_pool_max_size };
	sizes.at(1) = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_pool_max_size };
	sizes.at(2) = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_pool_max_size };
	sizes.at(3) = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_pool_max_size };
	sizes.at(4) = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_pool_max_size };
	sizes.at(5) = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_pool_max_size };
	sizes.at(6) = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_pool_max_size };
	sizes.at(7) = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_pool_max_size };
	sizes.at(8) = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_pool_max_size };
	sizes.at(9) = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_pool_max_size };
	sizes.at(10) = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptor_pool_max_size };
	sizes.at(11) = { VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, descriptor_pool_max_size };

	auto pool_create_info = vk_structures<VkDescriptorPoolCreateInfo> {}();
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.poolSizeCount = static_cast<std::uint32_t>(sizes.size());
	pool_create_info.pPoolSizes = sizes.data();
	pool_create_info.maxSets = descriptor_pool_max_size * static_cast<std::uint32_t>(sizes.size());

	verify(vkCreateDescriptorPool(vk_device, &pool_create_info, nullptr, &pool));

	std::vector<VkDescriptorSetLayout> desc_layouts(set_count * swapchain.image_count());
	for (std::size_t i = 0; i < desc_layouts.size() - 1; i += set_count) {
		desc_layouts[i] = layouts[0];
		desc_layouts[i + 1] = layouts[1];
	}

	VkDescriptorSetAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(desc_layouts.size());
	alloc_info.pSetLayouts = desc_layouts.data();

	descriptor_sets.resize(set_count * swapchain.image_count());
	vkAllocateDescriptorSets(vk_device, &alloc_info, descriptor_sets.data());
	for (std::size_t i = 0; i < descriptor_sets.size() - 1; i += set_count) {
		const auto& ubos = frame_ubos[i % swapchain.image_count()];

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
		write_sets[0].dstSet = descriptor_sets[i];
		write_sets[0].dstBinding = 0;
		write_sets[0].dstArrayElement = 0;
		write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[0].descriptorCount = 1;
		write_sets[0].pBufferInfo = &renderer_buffer;

		write_sets[1].dstSet = descriptor_sets[i];
		write_sets[1].dstBinding = 1;
		write_sets[1].dstArrayElement = 0;
		write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[1].descriptorCount = 1;
		write_sets[1].pBufferInfo = &camera_buffer;

		write_sets[2].dstSet = descriptor_sets[i];
		write_sets[2].dstBinding = 2;
		write_sets[2].dstArrayElement = 0;
		write_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[2].descriptorCount = 1;
		write_sets[2].pBufferInfo = &point_light_buffer;

		VkDescriptorImageInfo image_info = cast_to<Vulkan::Image>(viking_room->get_image()).get_descriptor_info();
		write_sets[3].dstSet = descriptor_sets[i + 1];
		write_sets[3].dstBinding = 0;
		write_sets[3].dstArrayElement = 0;
		write_sets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_sets[3].descriptorCount = 1;
		write_sets[3].pImageInfo = &image_info;

		vkUpdateDescriptorSets(vk_device, static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
	}
}

void GraphicsResource::expose_to_shaders(Disarray::Image& image)
{
	// const auto& descriptor_info = cast_to<Vulkan::Image>(image).get_descriptor_info();
	// Check if we can just add it to the descriptor sets

	// If not, reallocate
	// Else, add it
	// update descriptor sets?
}

void GraphicsResource::update_ubo()
{
	auto& current_uniform = frame_ubos[swapchain.get_current_frame()];
	current_uniform[0]->set_data(&uniform, sizeof(UBO));
	current_uniform[1]->set_data(&camera_ubo, sizeof(camera_ubo));
	current_uniform[2]->set_data(&lights, sizeof(PointLights));
}

GraphicsResource::~GraphicsResource()
{

	const auto& vk_device = supply_cast<Vulkan::Device>(device);
	Collections::for_each(layouts, [&vk_device](VkDescriptorSetLayout& layout) { vkDestroyDescriptorSetLayout(vk_device, layout, nullptr); });

	vkDestroyDescriptorPool(vk_device, pool, nullptr);
}

} // namespace Disarray::Vulkan
