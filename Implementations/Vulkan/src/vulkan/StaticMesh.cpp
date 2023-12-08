#include "DisarrayPCH.hpp"

#include <cstdint>
#include <string>

#include "core/Ensure.hpp"
#include "core/Log.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "graphics/MeshMaterial.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/StaticMesh.hpp"
#include "graphics/model_loaders/AssimpModelLoader.hpp"
#include "vulkan/MeshMaterial.hpp"
#include "vulkan/StaticMesh.hpp"

namespace Disarray {

template <> auto PimplDeleter<Assimp::Importer>::operator()(Assimp::Importer* ptr) noexcept -> void { delete ptr; }

namespace Vulkan {

	static constexpr uint32_t mesh_import_flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals
		| aiProcess_GenUVCoords |
		//		aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure
		| aiProcess_GlobalScale;

	StaticMesh::StaticMesh(const Disarray::Device& dev, const std::filesystem::path& path)
		: device(dev)
		, file_path(path)
	{
		importer = make_scope<Assimp::Importer, PimplDeleter<Assimp::Importer>>();
		importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		const aiScene* loaded_scene = importer->ReadFile(file_path.string(), mesh_import_flags);
		if (loaded_scene == nullptr) {
			Log::error("Mesh", "Failed to load mesh file: {0}", file_path.string());
			return;
		}

		scene = loaded_scene;

		if (!scene->HasMeshes()) {
			return;
		}

		uint32_t vertex_count = 0;
		uint32_t index_count = 0;

		submeshes.reserve(scene->mNumMeshes);
		for (std::uint32_t submesh_index = 0; submesh_index < scene->mNumMeshes; submesh_index++) {
			aiMesh* mesh = scene->mMeshes[submesh_index];

			StaticSubmesh& submesh = submeshes.emplace_back();
			submesh.base_vertex = vertex_count;
			submesh.base_index = index_count;
			submesh.material_index = mesh->mMaterialIndex;
			submesh.vertex_count = mesh->mNumVertices;
			submesh.index_count = mesh->mNumFaces * 3;
			submesh.mesh_name = mesh->mName.C_Str();

			vertex_count += mesh->mNumVertices;
			index_count += submesh.index_count;

			ensure(mesh->HasPositions(), "Meshes require positions.");
			ensure(mesh->HasNormals(), "Meshes require normals.");

			// Vertices
			auto& submesh_aabb = submesh.bounding_box;
			for (size_t i = 0; i < mesh->mNumVertices; i++) {
				ModelVertex vertex {};
				vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
				vertex.normals = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
				submesh_aabb.update(vertex.pos);

				if (mesh->HasTangentsAndBitangents()) {
					vertex.tangents = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					vertex.bitangents = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}

				if (mesh->HasTextureCoords(0)) {
					vertex.uvs = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}

				vertices.push_back(vertex);
			}

			// Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++) {
				ensure(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
				Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				indices.push_back(index);
			}
		}

		traverse_nodes(scene->mRootNode);

		for (const auto& submesh : submeshes) {
			const auto& submesh_aabb = submesh.bounding_box;
			const auto min = glm::vec3(submesh.transform * submesh_aabb.min_vector());
			const auto max = glm::vec3(submesh.transform * submesh_aabb.max_vector());

			bounding_box.update(min, max);
		}

		// Materials
		const auto& white_texture = Renderer::get_white_texture();
		if (scene->HasMaterials()) {
			materials.resize(scene->mNumMaterials);

			for (std::uint32_t i = 0; i < scene->mNumMaterials; i++) {
				auto* ai_material = scene->mMaterials[i];
				auto ai_material_name = ai_material->GetName();
				// convert to std::string
				std::string material_name = ai_material_name.C_Str();

				auto submesh_material = MeshMaterial::construct(device,
					MeshMaterialProperties {
						SingleShader::construct(device,
							{
								.path = FS::shader("basic_combined.glsl"),
								.optimize = false,
							}),
					});
				materials[i] = submesh_material;
				submesh_material->set("diffuse_map", white_texture);
				submesh_material->set("specular_map", white_texture);

				aiString ai_tex_path;
				std::uint32_t texture_count = ai_material->GetTextureCount(aiTextureType_DIFFUSE);
				glm::vec3 albedo_colour(0.8F);
				float emission = 0.0F;
				if (aiColor3D ai_colour; ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS) {
					albedo_colour = { ai_colour.r, ai_colour.g, ai_colour.b };
				}

				if (aiColor3D ai_emission; ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, ai_emission) == AI_SUCCESS) {
					emission = ai_emission.r;
				}

				submesh_material->set("pc.albedo_colour", albedo_colour);
				submesh_material->set("pc.emission", emission);

				float shininess {};
				float metalness {};
				if (ai_material->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS) {
					shininess = 80.0F; // Default value
				}

				if (ai_material->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS) {
					metalness = 0.0F;
				}

				float roughness = 1.0F - glm::sqrt(shininess / 100.0f);
				bool has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_tex_path) == AI_SUCCESS;
				bool fallback = !has_albedo_map;
				if (has_albedo_map) {
					Ref<Disarray::Texture> texture;
					if (const auto* ai_texture_embedded = scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
						DataBuffer buffer { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 4 };
						texture = Texture::construct(device,
							{
								.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = buffer,
								.debug_name = ai_tex_path.C_Str(),
							});
					} else {

						std::filesystem::path new_path = file_path;
						auto parent_path = new_path.parent_path();
						parent_path /= std::string(ai_tex_path.data);
						std::string texture_path = parent_path.string();
						texture = Texture::construct(device,
							{
								.path = texture_path,
								.debug_name = ai_tex_path.C_Str(),
							});
					}

					if (texture) {
						submesh_material->set("albedo_map", texture);
						submesh_material->set("pc.albedo_colour", glm::vec3(1.0F));
					} else {
						fallback = true;
					}
				}

				if (fallback) {
					submesh_material->set("albedo_map", white_texture);
				}

				// Normal maps
				auto has_normal_map = ai_material->GetTexture(aiTextureType_NORMALS, 0, &ai_tex_path) == AI_SUCCESS;
				fallback = !has_normal_map;
				if (has_normal_map) {
					Ref<Disarray::Texture> texture;
					if (const auto* ai_texture_embedded = scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
						texture = Texture::construct(device,
							{
								.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 3 },
								.debug_name = ai_tex_path.C_Str(),
							});
					} else {

						std::filesystem::path new_path = file_path;
						auto parent_path = new_path.parent_path();
						parent_path /= std::string(ai_tex_path.data);
						std::string texture_path = parent_path.string();
						texture = Texture::construct(device,
							{
								.path = texture_path,
								.debug_name = ai_tex_path.C_Str(),
							});
					}

					if (texture) {
						submesh_material->set("normal_map", texture);
						submesh_material->set("pc.use_normal_map", true);
					} else {
						fallback = true;
					}
				}

				if (fallback) {
					submesh_material->set("normal_map", white_texture);
					submesh_material->set("pc.use_normal_map", false);
				}

				// Roughness map
				bool has_roughness_map = ai_material->GetTexture(aiTextureType_SHININESS, 0, &ai_tex_path) == AI_SUCCESS;
				fallback = !has_roughness_map;
				if (has_roughness_map) {
					Ref<Disarray::Texture> texture;
					if (const auto* ai_texture_embedded = scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
						texture = Texture::construct(device,
							{
								.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 3 },
								.debug_name = ai_tex_path.C_Str(),
							});
					} else {

						std::filesystem::path new_path = file_path;
						auto parent_path = new_path.parent_path();
						parent_path /= std::string(ai_tex_path.data);
						std::string texture_path = parent_path.string();
						texture = Texture::construct(device,
							{
								.path = texture_path,
								.debug_name = ai_tex_path.C_Str(),
							});
					}

					if (texture) {
						submesh_material->set("roughness_map", texture);
						submesh_material->set("pc.roughness", 1.0F);
					} else {
						fallback = true;
					}
				}

				if (fallback) {
					submesh_material->set("roughness_map", white_texture);
					submesh_material->set("pc.roughness", roughness);
				}

				bool has_metalness_texture = false;
				for (std::uint32_t property_index = 0; property_index < ai_material->mNumProperties; property_index++) {
					if (auto* prop = ai_material->mProperties[property_index]; prop->mType == aiPTI_String) {
						uint32_t str_length = *reinterpret_cast<std::uint32_t*>(prop->mData);
						std::string str(prop->mData + 4, str_length);

						if (std::string key = prop->mKey.data; key == "$raw.ReflectionFactor|file") {
							Ref<Disarray::Texture> texture;
							if (const auto* ai_texture_embedded = scene->GetEmbeddedTexture(str.data())) {
								texture = Texture::construct(device,
									{
										.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
										.format = ImageFormat::RGB,
										.data_buffer
										= { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 3 },
										.debug_name = str,
									});
							} else {
								std::filesystem::path new_path = file_path;
								auto parent_path = new_path.parent_path();
								parent_path /= std::string(ai_tex_path.data);
								std::string texture_path = parent_path.string();
								texture = Texture::construct(device,
									{
										.path = texture_path,
										.debug_name = ai_tex_path.C_Str(),
									});
							}

							if (texture) {
								has_metalness_texture = true;
								submesh_material->set("metalness_map", texture);
								submesh_material->set("pc.metalness", 1.0F);
							} else {
								Log::error("Mesh", "Could not load texture: {0}", str);
							}
							break;
						}
					}
				}

				fallback = !has_metalness_texture;
				if (fallback) {
					submesh_material->set("metalness_map", white_texture);
					submesh_material->set("pc.metalness", metalness);
				}
			}
		} else {
			auto submesh_material = MeshMaterial::construct(device,
				MeshMaterialProperties {
					SingleShader::construct(device,
						{
							.path = FS::shader("basic_combined.glsl"),
							.optimize = false,
						}),
					"Default",
				});

			static constexpr auto default_roughness_and_albedo = 0.8F;

			submesh_material->set("pc.albedo_colour", glm::vec3(default_roughness_and_albedo));
			submesh_material->set("pc.emission", 0.0F);
			submesh_material->set("pc.metalness", 0.0F);
			submesh_material->set("pc.roughness", default_roughness_and_albedo);
			submesh_material->set("pc.use_normal_map", false);

			submesh_material->set("albedo_map", white_texture);
			submesh_material->set("normal_map", white_texture);
			submesh_material->set("metalness_map", white_texture);
			submesh_material->set("roughness_map", white_texture);
			materials.push_back(submesh_material);
		}

		vertex_buffer = VertexBuffer::construct(device,
			{
				.data = vertices.data(),
				.size = vertices.size() * sizeof(ModelVertex),
				.always_mapped = false,
			});
		index_buffer = IndexBuffer::construct(device,
			{
				.data = indices.data(),
				.size = indices.size() * sizeof(Index),
				.always_mapped = false,
			});
	}

	namespace {
		constexpr auto to_mat4_from_assimp(const aiMatrix4x4& matrix) -> glm::mat4
		{
			glm::mat4 result;
			result[0][0] = matrix.a1;
			result[1][0] = matrix.a2;
			result[2][0] = matrix.a3;
			result[3][0] = matrix.a4;
			result[0][1] = matrix.b1;
			result[1][1] = matrix.b2;
			result[2][1] = matrix.b3;
			result[3][1] = matrix.b4;
			result[0][2] = matrix.c1;
			result[1][2] = matrix.c2;
			result[2][2] = matrix.c3;
			result[3][2] = matrix.c4;
			result[0][3] = matrix.d1;
			result[1][3] = matrix.d2;
			result[2][3] = matrix.d3;
			result[3][3] = matrix.d4;
			return result;
		}
	} // namespace

	auto StaticMesh::traverse_nodes(aiNode* node, const glm::mat4& parent_transform, std::uint32_t level) -> void
	{
		if (node == nullptr) {
			return;
		}

		const glm::mat4 local_transform = to_mat4_from_assimp(node->mTransformation);
		const glm::mat4 transform = parent_transform * local_transform;
		node_map[node].resize(node->mNumMeshes);
		for (std::uint32_t i = 0; i < node->mNumMeshes; i++) {
			const std::uint32_t mesh = node->mMeshes[i];
			auto& submesh = submeshes[mesh];
			submesh.node_name = node->mName.C_Str();
			submesh.transform = transform;
			submesh.local_transform = local_transform;
			node_map[node][i] = mesh;
		}

		for (std::uint32_t i = 0; i < node->mNumChildren; i++) {
			traverse_nodes(node->mChildren[i], transform, level + 1);
		}
	}

} // namespace Vulkan

} // namespace Disarray
