#include "vulkan/MeshMaterial.hpp"

namespace Disarray::Vulkan {

MeshMaterial::MeshMaterial(const Disarray::Device& dev, MeshMaterialProperties properties)
	: Disarray::MeshMaterial(std::move(properties))
	, device(dev)
{
	recreate(false, {});
}

auto MeshMaterial::recreate_material(bool should_clean, const Extent& extent) -> void { }

} // namespace Disarray::Vulkan
