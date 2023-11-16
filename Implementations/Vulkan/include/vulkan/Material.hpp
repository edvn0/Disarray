#pragma once

#include "graphics/Device.hpp"
#include "graphics/Material.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UniformBuffer.hpp"

using VkDescriptorSet = struct VkDescriptorSet_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray::Vulkan {

class Material : public Disarray::Material {
public:
	Material(const Disarray::Device& dev, const MaterialProperties& properties);
	~Material() override;

	void recreate(bool, const Extent&) override;
	void force_recreation() override;

	void update_material(Disarray::Renderer&) override;
	void bind(const Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, FrameIndex index) const override;
	void write_textures(IGraphicsResource& resource) const override;

private:
	void recreate_material(bool should_clean);
	void recreate_descriptor_set_layout();

	const Disarray::Device& device;

	VkDescriptorSetLayout layout { nullptr };
	std::vector<VkDescriptorSet> frame_based_descriptor_sets {};
	bool needs_update { true };
	std::vector<Ref<Texture>> textures;
};

} // namespace Disarray::Vulkan
