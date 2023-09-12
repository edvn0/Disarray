#include "DisarrayPCH.hpp"

#include "graphics/ModelLoader.hpp"

#include <algorithm>
#include <string_view>

#include "core/Collections.hpp"
#include "graphics/ModelVertex.hpp"

namespace Disarray {

ModelLoader::ModelLoader(Scope<IModelImporter> input)
	: importer(std::move(input)) {

	};

auto ModelLoader::import_model(const std::filesystem::path& path) -> void
{
	auto&& meshes = importer->import(path);
	mesh_data = meshes;
}

auto ModelLoader::get_vertices_count(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).vertices.size();
}

auto ModelLoader::get_indices_size(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).indices.size() * sizeof(std::uint32_t);
}

auto ModelLoader::get_vertices_size(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).vertices.size() * sizeof(ModelVertex);
}

auto ModelLoader::get_indices_count(const std::string& mesh_identifier) const -> std::size_t { return mesh_data.at(mesh_identifier).indices.size(); }

} // namespace Disarray
