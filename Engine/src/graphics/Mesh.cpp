#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"
#include "graphics/ModelLoader.hpp"
#include "vulkan/Mesh.hpp"

namespace Disarray {

auto Mesh::construct(const Disarray::Device& device, Disarray::MeshProperties properties) -> Ref<Disarray::Mesh>
{
	return make_ref<Vulkan::Mesh>(device, std::move(properties));
}

auto Mesh::construct_scoped(const Disarray::Device& device, Disarray::MeshProperties properties) -> Scope<Disarray::Mesh>
{
	return make_scope<Vulkan::Mesh>(device, std::move(properties));
}

auto Mesh::construct_deferred(const Device& device, MeshProperties properties) -> std::future<Ref<Mesh>>
{
	return Vulkan::Mesh::construct_deferred(device, std::move(properties));
}

} // namespace Disarray
