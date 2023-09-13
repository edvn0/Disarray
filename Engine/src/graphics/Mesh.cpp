#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"

#include "vulkan/Mesh.hpp"

namespace Disarray {

auto Mesh::construct(const Disarray::Device& device, Disarray::MeshProperties properties) -> Ref<Disarray::Mesh>
{
	return make_ref<Vulkan::Mesh>(device, std::move(properties));
}

auto Mesh::construct_deferred(const Device& device, MeshProperties properties) -> std::future<Ref<Mesh>>
{
	return Vulkan::Mesh::construct_deferred(device, std::move(properties));
}

} // namespace Disarray
