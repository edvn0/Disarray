#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>

#include "graphics/Renderer.hpp"
#include "vulkan/Material.hpp"
#include "vulkan/Renderer.hpp"

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

void Material::recreate_material(bool should_clean) { }

void Material::recreate(bool, const Extent&) { }

void Material::force_recreation() { }

} // namespace Disarray::Vulkan
