#pragma once

#include "graphics/Device.hpp"
#include "graphics/Material.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UniformBuffer.hpp"
#include "vulkan/GraphicsResource.hpp"

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
	void recreate_descriptor_set_layout(Vulkan::GraphicsResource&);

	const Disarray::Device& device;

	std::vector<VkDescriptorSetLayout> layouts {};
	std::unordered_map<FrameIndex, std::vector<VkDescriptorSet>> frame_based_descriptor_sets {};
	bool needs_update { true };
};

class POCMaterial : public Disarray::POCMaterial {
public:
	POCMaterial(const Disarray::Device& dev, POCMaterialProperties properties);
	~POCMaterial() override = default;

private:
	const Disarray::Device& device;
};

} // namespace Disarray::Vulkan
