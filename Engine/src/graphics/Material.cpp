#include "vulkan/Material.hpp"

#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"

namespace Disarray {

auto Material::construct(const Disarray::Device& device, MaterialProperties properties) -> Ref<Disarray::Material>
{
	return make_ref<Vulkan::Material>(device, properties);
}

} // namespace Disarray
