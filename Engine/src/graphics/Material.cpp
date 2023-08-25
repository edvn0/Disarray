#include "vulkan/Material.hpp"

#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"

namespace Disarray {

Ref<Material> Material::construct(const Disarray::Device& device, const MaterialProperties& properties)
{
	return make_ref<Vulkan::Material>(device, properties);
}

} // namespace Disarray
