#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"

#include "vulkan/Mesh.hpp"

namespace Disarray {

auto Mesh::construct(const Disarray::Device& device, Disarray::MeshProperties properties) -> Ref<Disarray::Mesh>
{
	return make_ref<Vulkan::Mesh>(device, std::move(properties));
}

} // namespace Disarray
