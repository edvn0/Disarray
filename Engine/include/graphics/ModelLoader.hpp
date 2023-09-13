#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/Collections.hpp"
#include "core/PointerDefinition.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

struct Submesh {
	std::vector<ModelVertex> vertices {};
	std::vector<uint32_t> indices {};
	std::vector<Disarray::TextureProperties> texture_properties {};
	std::vector<Ref<Disarray::Texture>> textures {};
};

using ImportedSubmesh = Submesh;
using ImportedMesh = Collections::StringMap<ImportedSubmesh>;

struct IModelImporter {
	virtual ~IModelImporter() = default;
	virtual auto import(const std::filesystem::path&) -> ImportedMesh = 0;
};

class ModelLoader {
public:
	explicit ModelLoader(Scope<IModelImporter>);
	explicit ModelLoader(Scope<IModelImporter>, const std::filesystem::path&);
	void import_model(const std::filesystem::path&);
	void construct_textures(const Device&);

	[[nodiscard]] auto get_vertices(const std::string& mesh_identifier) const -> const auto& { return mesh_data.at(mesh_identifier).vertices; }
	[[nodiscard]] auto get_indices(const std::string& mesh_identifier) const -> const auto& { return mesh_data.at(mesh_identifier).indices; }
	[[nodiscard]] auto get_textures(const std::string& mesh_identifier) const -> const auto& { return mesh_data.at(mesh_identifier).textures; }

	[[nodiscard]] auto get_vertices_size(const std::string& mesh_identifier) const -> std::size_t;
	[[nodiscard]] auto get_indices_size(const std::string& mesh_identifier) const -> std::size_t;
	[[nodiscard]] auto get_textures_size(const std::string& mesh_identifier) const -> std::size_t;

	[[nodiscard]] auto get_vertices_count(const std::string& mesh_identifier) const -> std::size_t;
	[[nodiscard]] auto get_indices_count(const std::string& mesh_identifier) const -> std::size_t;
	[[nodiscard]] auto get_textures_count(const std::string& mesh_identifier) const -> std::size_t;

private:
	Scope<IModelImporter> importer { nullptr };
	ImportedMesh mesh_data {};
};

} // namespace Disarray
