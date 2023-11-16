#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "graphics/CommandExecutor.hpp"
#include "graphics/Renderer.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/GraphicsResource.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Material.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/Renderer.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::Vulkan {

Material::Material(const Disarray::Device& dev, const Disarray::MaterialProperties& properties)
	: Disarray::Material(properties)
	, device(dev)
{
	recreate_material(false);
};

Material::~Material() = default;

void Material::update_material(Disarray::Renderer& renderer)
{
	if (!needs_update) {
		return;
	}

	recreate_descriptor_set_layout();

	VkDescriptorSetAllocateInfo allocation_info {};
	allocation_info.pSetLayouts = &layout;
	allocation_info.descriptorSetCount = 1;

	frame_based_descriptor_sets.clear();
	frame_based_descriptor_sets.resize(1);
	auto& graphics_resource = cast_to<Vulkan::GraphicsResource>(renderer.get_graphics_resource());
	graphics_resource.allocate_descriptor_sets(allocation_info, frame_based_descriptor_sets);

	needs_update = false;
}

void Material::write_textures(IGraphicsResource& resource) const
{
	for (auto i = FrameIndex { 0 }; i < resource.get_image_count(); i++) {
		auto* set = resource.get_descriptor_set(i, DescriptorSet(2));

		std::vector<VkWriteDescriptorSet> write_descriptors {};
		std::uint32_t binding = 3;
		Collections::for_each_unwrapped(props.textures, [&](const auto&, const auto& texture) mutable {
			const auto& vk_image = cast_to<Vulkan::Image>(texture->get_image());
			auto& write_descriptor_set = write_descriptors.emplace_back();
			write_descriptor_set.descriptorCount = 1;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.dstSet = set;
			write_descriptor_set.dstBinding = binding++;
			write_descriptor_set.pImageInfo = &vk_image.get_descriptor_info();
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		});

		vkUpdateDescriptorSets(
			supply_cast<Vulkan::Device>(device), static_cast<std::uint32_t>(write_descriptors.size()), write_descriptors.data(), 0, nullptr);
	}
}

void Material::bind(const Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, FrameIndex index) const
{
	auto* command_buffer = supply_cast<Vulkan::CommandExecutor>(executor);
	auto* pipeline_layout = cast_to<Vulkan::Pipeline>(pipeline).get_layout();

	auto* descriptor_set = frame_based_descriptor_sets.at(0);
	std::array set { descriptor_set };

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, set.data(), 0, nullptr);
}

void Material::recreate_material(bool should_clean) { }

void Material::recreate_descriptor_set_layout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings {};
	Collections::for_each<true>(props.textures, [&binds = bindings](auto&, std::size_t i) {
		auto& binding = binds.emplace_back();

		binding.binding = static_cast<std::uint32_t>(i) + 3;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	});

	VkDescriptorSetLayoutCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
	create_info.pBindings = bindings.data();

	verify(vkCreateDescriptorSetLayout(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &layout));
}

void Material::recreate(bool should_clean, const Extent&) { recreate_material(should_clean); }

void Material::force_recreation() { recreate_material(true); }

} // namespace Disarray::Vulkan
