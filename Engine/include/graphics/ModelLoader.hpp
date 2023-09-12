#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/Collections.hpp"
#include "core/Types.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "graphics/ModelVertex.hpp"

namespace Disarray {

struct Submesh {
	std::vector<ModelVertex> vertices {};
	std::vector<uint32_t> indices {};
};

using ImportedSubmesh = Submesh;
using ImportedMesh = Collections::StringMap<ImportedSubmesh>;

struct IModelImporter {
	virtual ~IModelImporter() = default;
	virtual auto import(const std::filesystem::path&) -> ImportedMesh = 0;
};

class ModelLoader {
public:
	[[nodiscard]] auto get_vertices(const std::string& mesh_identifier) const -> const auto& { return mesh_data.at(mesh_identifier).vertices; }
	[[nodiscard]] auto get_indices(const std::string& mesh_identifier) const -> const auto& { return mesh_data.at(mesh_identifier).indices; }

	[[nodiscard]] auto get_vertices_size(const std::string& mesh_identifier) const -> std::size_t;
	[[nodiscard]] auto get_indices_size(const std::string& mesh_identifier) const -> std::size_t;

	[[nodiscard]] auto get_vertices_count(const std::string& mesh_identifier) const -> std::size_t;
	[[nodiscard]] auto get_indices_count(const std::string& mesh_identifier) const -> std::size_t;

	explicit ModelLoader(Scope<IModelImporter>);
	void import_model(const std::filesystem::path&);

private:
	Scope<IModelImporter> importer { nullptr };
	ImportedMesh mesh_data {};
};

} // namespace Disarray
