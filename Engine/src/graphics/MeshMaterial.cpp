#include "graphics/MeshMaterial.hpp"

#include "graphics/StaticMesh.hpp"
#include "vulkan/MeshMaterial.hpp"

namespace Disarray {

auto MeshMaterial::construct(const Disarray::Device& device, MeshMaterialProperties properties) -> Ref<Disarray::MeshMaterial>
{
	return make_ref<Vulkan::MeshMaterial>(device, properties);
}

auto MeshMaterial::construct_scoped(const Disarray::Device& device, MeshMaterialProperties properties) -> Scope<Disarray::MeshMaterial>
{
	return make_scope<Vulkan::MeshMaterial>(device, properties);
}

} // namespace Disarray
