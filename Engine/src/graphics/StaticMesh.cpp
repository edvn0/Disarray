#include "graphics/StaticMesh.hpp"

#include <filesystem>

#include "vulkan/StaticMesh.hpp"

namespace Disarray {

auto StaticMesh::construct(const Disarray::Device& device, StaticMeshProperties properties) -> Ref<Disarray::StaticMesh>
{
	return make_ref<Vulkan::StaticMesh>(device, properties.path);
}

auto StaticMesh::construct_scoped(const Disarray::Device& device, StaticMeshProperties properties) -> Scope<Disarray::StaticMesh>
{
	return make_scope<Vulkan::StaticMesh>(device, properties.path);
}

auto StaticMesh::construct(const Disarray::Device& device, const std::filesystem::path& path) -> Ref<Disarray::StaticMesh>
{
	return make_ref<Vulkan::StaticMesh>(device, path);
}

} // namespace Disarray
