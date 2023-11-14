#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>

#include "graphics/Renderer.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Material.hpp"
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
	// Update materials!

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

void Material::recreate_material(bool should_clean) { }

void Material::recreate(bool, const Extent&) { }

void Material::force_recreation() { }

} // namespace Disarray::Vulkan
