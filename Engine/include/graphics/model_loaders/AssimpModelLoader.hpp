#pragma once

#include <glm/glm.hpp>

#include <filesystem>

#include "graphics/ModelLoader.hpp"
#include "graphics/Pipeline.hpp"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Disarray {

struct AssimpModelLoader final : public IModelImporter {
	explicit AssimpModelLoader(const glm::mat4& rot = glm::identity<glm::mat4>())
		: initial_rotation(rot) {};
	auto import_model(const std::filesystem::path& path, ImportFlag) -> ImportedMesh final;

private:
	static auto process_mesh(aiMesh* mesh, const std::filesystem::path& base_directory, const aiScene* scene) -> Submesh;
	static auto process_current_node(ImportedMesh& mesh_map, const std::filesystem::path& base_directory, aiNode* current_node, const aiScene* scene);

	glm::mat4 initial_rotation { 1.0F };
};

} // namespace Disarray
