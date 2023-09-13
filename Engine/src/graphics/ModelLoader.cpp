#include "DisarrayPCH.hpp"

#include "graphics/ModelLoader.hpp"

#include "graphics/Device.hpp"
#include "graphics/ModelVertex.hpp"

namespace Disarray {

ModelLoader::ModelLoader(Scope<IModelImporter> input)
	: importer(std::move(input)) {

	};

ModelLoader::ModelLoader(Scope<IModelImporter> input, const std::filesystem::path& path)
	: importer(std::move(input))
{
	import_model(path);
};

auto ModelLoader::import_model(const std::filesystem::path& path) -> void
{
	auto&& meshes = importer->import(path);
	mesh_data = meshes;
}

void ModelLoader::construct_textures(const Disarray::Device& device)
{
	for (auto& [key, value] : mesh_data) {
		for (const auto& props : value.texture_properties) {
			value.textures.push_back(Texture::construct(device,
				{
					.format = ImageFormat::SRGB,
					.path = props.path,
					.debug_name = props.debug_name,
				}));
		}
	}
}

auto ModelLoader::get_indices_size(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).indices.size() * sizeof(std::uint32_t);
}

auto ModelLoader::get_vertices_size(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).vertices.size() * sizeof(ModelVertex);
}

auto ModelLoader::get_textures_size(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).textures.size() * sizeof(ModelVertex);
}

auto ModelLoader::get_vertices_count(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).vertices.size();
}

auto ModelLoader::get_indices_count(const std::string& mesh_identifier) const -> std::size_t { return mesh_data.at(mesh_identifier).indices.size(); }

auto ModelLoader::get_textures_count(const std::string& mesh_identifier) const -> std::size_t
{
	return mesh_data.at(mesh_identifier).textures.size();
}

} // namespace Disarray
