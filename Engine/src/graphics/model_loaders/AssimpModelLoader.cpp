#include "graphics/model_loaders/AssimpModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <span>

#include "core/Log.hpp"

namespace Disarray {

auto process_mesh(aiMesh* mesh, const aiScene* scene) -> Submesh
{
	std::span mesh_vertices { mesh->mVertices, mesh->mNumVertices };
	std::span mesh_normals { mesh->mNormals, mesh->mNumVertices };
	std::span mesh_uvs { mesh->mTextureCoords, mesh->mNumVertices };
	std::span mesh_colours { mesh->mColors, mesh->mNumVertices };

	const auto has_colours = mesh_colours.empty();
	const auto has_normals = mesh_normals.empty();

	std::unordered_map<ModelVertex, uint32_t> unique_vertices {};
	std::vector<ModelVertex> vertices;
	std::vector<std::uint32_t> indices;

	for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
		ModelVertex model_vertex {};
		model_vertex.pos = { mesh_vertices[i].x, mesh_vertices[i].y, mesh_vertices[i].z };
		model_vertex.uvs = { mesh_uvs[0][i].x, mesh_uvs[0][i].y };
		model_vertex.normals = has_normals ? glm::vec3 { mesh_normals[i].x, mesh_normals[i].y, mesh_normals[i].z } : glm::vec3 {};
		model_vertex.color = has_colours? glm::vec4 {
			mesh_colours[i]->r,
			mesh_colours[i]->g,
			mesh_colours[i]->b,
			mesh_colours[i]->a,
		}:glm::vec4{};

		if (!unique_vertices.contains(model_vertex)) {
			const auto current_size = static_cast<std::uint32_t>(vertices.size());
			unique_vertices.try_emplace(model_vertex, current_size);
			vertices.push_back(model_vertex);
		}

		indices.push_back(unique_vertices[model_vertex]);
	}

	// process material
	if (mesh->mMaterialIndex >= 0) {
		// We'll get here
	}

	return Submesh(vertices, indices);
}

auto process_current_node(ImportedMesh& mesh_map, aiNode* current_node, const aiScene* scene)
{
	if (scene == nullptr) {
		return;
	}

	std::span node_meshes { current_node->mMeshes, current_node->mNumMeshes };
	std::span scene_meshes { scene->mMeshes, scene->mNumMeshes };

	for (const auto& mesh_index : node_meshes) {
		aiMesh* ai_mesh = scene_meshes[mesh_index];
		const aiString mesh_name = ai_mesh->mName;
		std::string name(mesh_name.length, '\0');

		std::memcpy(name.data(), mesh_name.C_Str(), mesh_name.length);

		name.resize(mesh_name.length);
		mesh_map.try_emplace(name, process_mesh(ai_mesh, scene));
	}
	// then do the same for each of its children

	std::span children { current_node->mChildren, current_node->mNumChildren };
	for (const auto& child : children) {
		process_current_node(mesh_map, child, scene);
	}
}

auto AssimpModelLoader::import(const std::filesystem::path& path) -> ImportedMesh
{
	ImportedMesh output {};

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

	if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) || (scene->mRootNode == nullptr)) {
		throw CouldNotLoadModelException(fmt::format("Error: {}", importer.GetErrorString()));
	}
	static const auto directory = std::filesystem::path { "Assets" };

	process_current_node(output, scene->mRootNode, scene);

	return output;
}

} // namespace Disarray