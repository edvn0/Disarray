#pragma once

#include "graphics/Device.hpp"
#include "graphics/Material.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UniformBuffer.hpp"

namespace Disarray::Vulkan {

class Material : public Disarray::Material {
public:
	Material(const Device& dev, const MaterialProperties& properties);
	~Material() override;

	void recreate(bool, const Extent&) override;
	void force_recreation() override;

	void update_material(Disarray::Renderer&) override;

	auto get_descriptor_layouts() const { return layout; }

	const auto& get_descriptor_set(std::uint32_t) const { return descriptor_set; }

private:
	void recreate_material(bool should_clean);

	const Device& device;
	VkDescriptorSetLayout layout {};
	VkDescriptorSet descriptor_set {};

	bool needs_update { true };
	std::vector<Ref<Texture>> textures;
	std::vector<Ref<UniformBuffer>> buffers;
};

} // namespace Disarray::Vulkan
