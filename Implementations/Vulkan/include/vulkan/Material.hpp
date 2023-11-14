#pragma once

#include "graphics/Device.hpp"
#include "graphics/Material.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/UniformBuffer.hpp"

namespace Disarray::Vulkan {

class Material : public Disarray::Material {
public:
	Material(const Disarray::Device& dev, const MaterialProperties& properties);
	~Material() override;

	void recreate(bool, const Extent&) override;
	void force_recreation() override;

	void update_material(Disarray::Renderer&) override;

	void write_textures(IGraphicsResource& resource) const override;

private:
	void recreate_material(bool should_clean);

	const Disarray::Device& device;

	bool needs_update { true };
	std::vector<Ref<Texture>> textures;
};

} // namespace Disarray::Vulkan
