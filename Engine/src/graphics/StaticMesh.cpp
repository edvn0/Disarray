#include "graphics/StaticMesh.hpp"

#include <filesystem>

#include "core/filesystem/FileIO.hpp"
#include "vulkan/StaticMesh.hpp"

namespace Disarray {

auto StaticMesh::construct(const Disarray::Device& device, StaticMeshProperties properties) -> Ref<Disarray::StaticMesh>
{
	if (!FS::exists(properties.path)) {
		return nullptr;
	}
	return make_ref<Vulkan::StaticMesh>(device, properties.path);
}

auto StaticMesh::construct_scoped(const Disarray::Device& device, StaticMeshProperties properties) -> Scope<Disarray::StaticMesh>
{
	if (!FS::exists(properties.path)) {
		return nullptr;
	}
	return make_scope<Vulkan::StaticMesh>(device, properties.path);
}

auto StaticMesh::construct(const Disarray::Device& device, const std::filesystem::path& path) -> Ref<Disarray::StaticMesh>
{
	if (!FS::exists(path)) {
		return nullptr;
	}
	return make_ref<Vulkan::StaticMesh>(device, path);
}

} // namespace Disarray
