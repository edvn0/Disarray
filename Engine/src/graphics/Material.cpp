#include "vulkan/Material.hpp"

#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"

namespace Disarray {

auto Material::construct(const Disarray::Device& device, const MaterialProperties& properties) -> Ref<Material>
{
	return make_ref<Vulkan::Material>(device, properties);
}

} // namespace Disarray
