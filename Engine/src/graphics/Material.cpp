#include "vulkan/Material.hpp"

#include "graphics/Material.hpp"

namespace Disarray {

auto Material::construct(const Disarray::Device& device, MaterialProperties properties) -> Ref<Disarray::Material>
{
	return make_ref<Vulkan::Material>(device, properties);
}

auto Material::construct_scoped(const Disarray::Device& device, MaterialProperties properties) -> Scope<Disarray::Material>
{
	return make_scope<Vulkan::Material>(device, properties);
}

} // namespace Disarray
