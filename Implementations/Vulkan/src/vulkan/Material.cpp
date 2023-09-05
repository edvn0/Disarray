#include "vulkan/Material.hpp"

#include <vulkan/vulkan.h>

#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Material.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Texture.hpp"
#include "vulkan/UniformBuffer.hpp"

namespace Disarray::Vulkan {

Material::Material(const Disarray::Device& dev, const Disarray::MaterialProperties& properties)
	: Disarray::Material(properties)
	, device(dev)
{
	recreate_material(false);
};

void Material::update_material(Disarray::Renderer& renderer)
{
	if (!needs_update)
		return;

	/*VkDescriptorSetAllocateInfo allocation_info {};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &layout;
	descriptor_set = cast_to<Vulkan::Renderer>(renderer).allocate_descriptor_set(allocation_info);

	std::array<VkWriteDescriptorSet, 2> write_descriptor {};
	write_descriptor[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor[0].descriptorCount = 1;
	write_descriptor[0].dstBinding = 0;
	write_descriptor[0].dstSet = descriptor_set;
	write_descriptor[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor[0].pBufferInfo = &cast_to<Vulkan::UniformBuffer>(renderer.get_ubo()).get_buffer_info();

	write_descriptor[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor[1].descriptorCount = 1;
	write_descriptor[1].dstBinding = 1;
	write_descriptor[1].dstSet = descriptor_set;
	write_descriptor[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	auto image_info = cast_to<Vulkan::Image>(renderer.get_texture_cache().get("viking_room")->get_image()).get_descriptor_info();
	write_descriptor[1].pImageInfo = &image_info;

	vkUpdateDescriptorSets(
		supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_descriptor.size()), write_descriptor.data(), 0, 0);
	*/
	needs_update = false;
}

void Material::recreate_material(bool should_clean)
{
	std::array<VkDescriptorSetLayoutBinding, 2> binds {};
	binds[0].binding = 0;
	binds[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	binds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binds[0].pImmutableSamplers = nullptr;
	binds[0].descriptorCount = 1;

	binds[1].binding = 1;
	binds[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binds[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binds[1].pImmutableSamplers = nullptr;
	binds[1].descriptorCount = 1;

	auto create_info = vk_structures<VkDescriptorSetLayoutCreateInfo> {}();
	create_info.bindingCount = static_cast<std::uint32_t>(binds.size());
	create_info.pBindings = binds.data();

	vkCreateDescriptorSetLayout(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &layout);
}

void Material::recreate(bool, const Extent&) { }

void Material::force_recreation() { }

Material::~Material() { vkDestroyDescriptorSetLayout(supply_cast<Vulkan::Device>(device), layout, nullptr); }

} // namespace Disarray::Vulkan
