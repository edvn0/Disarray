#include "graphics/model_loaders/AssimpModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <magic_enum.hpp>

#include <filesystem>
#include <span>

#include "core/Formatters.hpp"
#include "core/Log.hpp"
#include "graphics/Texture.hpp"

namespace Disarray {

auto load_texture(aiMaterial* mat, const std::filesystem::path& base_directory, aiTextureType type, std::string_view type_name)
	-> std::vector<TextureProperties>
{
	std::vector<Disarray::TextureProperties> properties;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		const auto as_path = std::filesystem::relative(base_directory / std::filesystem::path { str.C_Str() });

		if (!std::filesystem::exists(as_path)) {
			Log::info("AssimpModelLoader", "Could not find texture with path {}", as_path);
			continue;
		}

		TextureProperties& props = properties.emplace_back();
		props.debug_name = str.C_Str();
		props.path = as_path;
	}
	return properties;
}

auto process_mesh(aiMesh* mesh, const std::filesystem::path& base_directory, const aiScene* scene) -> Submesh
{
	std::span mesh_vertices { mesh->mVertices, mesh->mNumVertices };
	std::span mesh_normals { mesh->mNormals, mesh->mNumVertices };
	std::span mesh_uvs { mesh->mTextureCoords[0], mesh->mNumVertices };
	std::span mesh_colours { mesh->mColors[0], mesh->mNumVertices };

	const auto has_colours = mesh_colours.empty();
	const auto has_normals = mesh_normals.empty();

	std::unordered_map<ModelVertex, uint32_t> unique_vertices {};
	std::vector<ModelVertex> vertices;
	std::vector<std::uint32_t> indices;
	std::vector<Disarray::TextureProperties> textures;

	for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
		ModelVertex model_vertex {};
		model_vertex.pos = { mesh_vertices[i].x, mesh_vertices[i].y, mesh_vertices[i].z };
		model_vertex.uvs = { mesh_uvs[i].x, mesh_uvs[i].y };
		model_vertex.normals = has_normals ? glm::vec3 { mesh_normals[i].x, mesh_normals[i].y, mesh_normals[i].z } : glm::vec3 {};

		model_vertex.color = glm::vec4 {};
		if (has_colours) {
			model_vertex.color = glm::vec4 {
				mesh_colours[i].r,
				mesh_colours[i].g,
				mesh_colours[i].b,
				mesh_colours[i].a,
			};
		}

		if (!unique_vertices.contains(model_vertex)) {
			const auto current_size = static_cast<std::uint32_t>(vertices.size());
			unique_vertices.try_emplace(model_vertex, current_size);
			vertices.push_back(model_vertex);
		}

		indices.push_back(unique_vertices[model_vertex]);
	}

	// process material

	const auto has_materials = mesh->mMaterialIndex > 0;
	Log::info("AssimpModelLoader", "Material index: {}", mesh->mMaterialIndex);
	if (has_materials) {
		std::span materials { scene->mMaterials, scene->mNumMaterials };
		aiMaterial* material = materials[mesh->mMaterialIndex];

		constexpr auto texture_types = magic_enum::enum_values<aiTextureType>();

		for (const auto& type : texture_types) {
			auto texture_type_properties = load_texture(material, base_directory, type, "");
			textures.insert(textures.end(), texture_type_properties.begin(), texture_type_properties.end());
		}
	}

	return Submesh(vertices, indices, textures);
}

auto process_current_node(ImportedMesh& mesh_map, const std::filesystem::path& base_directory, aiNode* current_node, const aiScene* scene)
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
		mesh_map.try_emplace(name, process_mesh(ai_mesh, base_directory, scene));
	}
	// then do the same for each of its children

	std::span children { current_node->mChildren, current_node->mNumChildren };
	for (const auto& child : children) {
		process_current_node(mesh_map, base_directory, child, scene);
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
	const auto base_directory = path.parent_path();
	process_current_node(output, base_directory, scene->mRootNode, scene);

	return output;
}

} // namespace Disarray