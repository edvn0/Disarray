#include "DisarrayPCH.hpp"

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

void Renderer::initialise_descriptors()
{
	auto vk_device = supply_cast<Vulkan::Device>(device);

	TextureCacheCreationProperties texture_properties { .key = "viking", .debug_name = "viking" };
	texture_properties.path = "Assets/Textures/viking_room.png";
	texture_properties.format = ImageFormat::SRGB;
	auto viking_room = texture_cache.put(texture_properties);

	auto default_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	default_binding.descriptorCount = 1;
	default_binding.binding = 0;
	default_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	default_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	auto image_binding = vk_structures<VkDescriptorSetLayoutBinding> {}();
	image_binding.descriptorCount = 1;
	image_binding.binding = 0;
	image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	auto layout_create_info = vk_structures<VkDescriptorSetLayoutCreateInfo> {}();
	layout_create_info.bindingCount = 1;
	layout_create_info.pBindings = &default_binding;

	VkDescriptorSetLayout default_layout;
	VkDescriptorSetLayout image_layout;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &default_layout));

	layout_create_info.pBindings = &image_binding;
	verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, &image_layout));

	layouts = { default_layout, image_layout };

	std::array<VkDescriptorPoolSize, 12> sizes;
	sizes[0] = { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 };
	sizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 };
	sizes[2] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 };
	sizes[3] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 };
	sizes[4] = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 };
	sizes[5] = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 };
	sizes[6] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 };
	sizes[7] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 };
	sizes[8] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 };
	sizes[9] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 };
	sizes[10] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 };
	sizes[11] = { VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK, 1000 };

	auto pool_create_info = vk_structures<VkDescriptorPoolCreateInfo> {}();
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.poolSizeCount = static_cast<std::uint32_t>(sizes.size());
	pool_create_info.pPoolSizes = sizes.data();
	pool_create_info.maxSets = 1000 * static_cast<std::uint32_t>(sizes.size());

	verify(vkCreateDescriptorPool(vk_device, &pool_create_info, nullptr, &pool));

	std::vector<VkDescriptorSetLayout> desc_layouts(set_count * swapchain.image_count());
	for (std::size_t i = 0; i < desc_layouts.size() - 1; i += 2) {
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
	for (std::size_t i = 0; i < descriptor_sets.size() - 1; i += 2) {
		VkDescriptorBufferInfo buffer_info {};
		buffer_info.buffer = supply_cast<Vulkan::UniformBuffer>(*frame_ubos[i % swapchain.image_count()]);
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UBO);

		auto write_sets = vk_structures<VkWriteDescriptorSet, 2> {}.multiple();
		write_sets[0].dstSet = descriptor_sets[i];
		write_sets[0].dstBinding = 0;
		write_sets[0].dstArrayElement = 0;
		write_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_sets[0].descriptorCount = 1;
		write_sets[0].pBufferInfo = &buffer_info;

		VkDescriptorImageInfo image_info = cast_to<Vulkan::Image>(viking_room->get_image()).get_descriptor_info();
		write_sets[1].dstSet = descriptor_sets[i + 1];
		write_sets[1].dstBinding = 0;
		write_sets[1].dstArrayElement = 0;
		write_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_sets[1].descriptorCount = 1;
		write_sets[1].pImageInfo = &image_info;

		vkUpdateDescriptorSets(vk_device, static_cast<std::uint32_t>(write_sets.size()), write_sets.data(), 0, nullptr);
	}
}

void Renderer::expose_to_shaders(Disarray::Image& image)
{
	const auto& descriptor_info = cast_to<Vulkan::Image>(image).get_descriptor_info();
	// Check if we can just add it to the descriptor sets

	// If not, reallocate
	// Else, add it
	// update descriptor sets?
}

} // namespace Disarray::Vulkan
