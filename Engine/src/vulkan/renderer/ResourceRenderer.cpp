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

	void Renderer::initialise_descriptors()
	{
		auto vk_device = supply_cast<Vulkan::Device>(device);

		TextureProperties texture_properties { .debug_name = "viking" };
		texture_properties.path = "Assets/Textures/viking_room.png";
		texture_properties.format = ImageFormat::SRGB;
		auto viking_room = texture_cache.emplace_back(Texture::construct(device, texture_properties));

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

		layouts.resize(1);
		verify(vkCreateDescriptorSetLayout(vk_device, &layout_create_info, nullptr, layouts.data()));

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

		std::vector<VkDescriptorSetLayout> desc_layouts(swapchain.image_count(), layouts[0]);
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(swapchain.image_count());
		alloc_info.pSetLayouts = desc_layouts.data();

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

	void Renderer::expose_to_shaders(Disarray::Image& image)
	{
		const auto& descriptor_info = cast_to<Vulkan::Image>(image).get_descriptor_info();
		// Check if we can just add it to the descriptor sets

		// If not, reallocate
		// Else, add it
		// update descriptor sets?
	}

} // namespace Disarray::Vulkan
