#include "DisarrayPCH.hpp"

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <core/filesystem/AssetLocations.hpp>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "core/Ensure.hpp"
#include "core/Log.hpp"
#include "core/ThreadPool.hpp"
#include "core/Types.hpp"
#include "core/exceptions/GeneralExceptions.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/model_loaders/AssimpModelLoader.hpp"
#include "scene/SceneRenderer.hpp"
#include "vulkan/Mesh.hpp"

namespace Disarray {

template <> auto PimplDeleter<Assimp::Importer>::operator()(Assimp::Importer* ptr) noexcept -> void { delete ptr; }

struct AssimpLogStream : public Assimp::LogStream {
	static void initalize()
	{
		if (Assimp::DefaultLogger::isNullLogger()) {
			Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
			Assimp::DefaultLogger::get()->attachStream(
				new AssimpLogStream, Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err);
		}
	}

	virtual void write(const char* message) override
	{
		std::string msg(message);
		if (!msg.empty() && msg[msg.length() - 1] == '\n') {
			msg.erase(msg.length() - 1);
		}
		if (strncmp(message, "Debug", 5) == 0) {
			Log::trace("Assimp", msg);
		} else if (strncmp(message, "Info", 4) == 0) {
			Log::info("Assimp", msg);
		} else if (strncmp(message, "Warn", 4) == 0) {
			Log::warn("Assimp", msg);
		} else {
			Log::error("Assimp", msg);
		}
	}
};

namespace Vulkan {

	Mesh::~Mesh() = default;

	Mesh::Mesh(const Disarray::Device& dev, Disarray::MeshProperties properties)
		: Disarray::Mesh(std::move(properties))
		, device(dev)
	{
		load_and_initialise_model();
	}

	Mesh::Mesh(const Disarray::Device& dev, Scope<Disarray::VertexBuffer> vertices, Scope<Disarray::IndexBuffer> indices,
		const std::vector<Ref<Disarray::Texture>>& textures)
		: device(dev)
		, vertex_buffer(std::move(vertices))
		, index_buffer(std::move(indices))
		, mesh_textures(textures)
	{
	}

	void Mesh::load_and_initialise_model(const ImportedMesh& imported)
	{
		if (imported.size() == 1) {
			const auto& kv = *imported.begin();
			auto&& [key, loaded_submesh] = kv;
			vertex_buffer = VertexBuffer::construct_scoped(device,
				{
					.data = loaded_submesh.data<ModelVertex>(),
					.size = loaded_submesh.size<ModelVertex>(),
					.count = loaded_submesh.count<ModelVertex>(),
				});
			index_buffer = IndexBuffer::construct_scoped(device,
				{
					.data = loaded_submesh.data<std::uint32_t>(),
					.size = loaded_submesh.size<std::uint32_t>(),
					.count = loaded_submesh.count<std::uint32_t>(),
				});
			aabb = loaded_submesh.aabb;
			mesh_textures = loaded_submesh.textures;

			return;
		}

		for (const auto& mesh_data = imported; const auto& [key, loaded_submesh] : mesh_data) {
			auto vb = VertexBuffer::construct_scoped(device,
				{
					.data = loaded_submesh.data<ModelVertex>(),
					.size = loaded_submesh.size<ModelVertex>(),
					.count = loaded_submesh.count<ModelVertex>(),
				});
			auto ib = IndexBuffer::construct_scoped(device,
				{
					.data = loaded_submesh.data<std::uint32_t>(),
					.size = loaded_submesh.size<std::uint32_t>(),
					.count = loaded_submesh.count<std::uint32_t>(),
				});

			auto submesh = Scope<Vulkan::Mesh> { new Vulkan::Mesh { device, std::move(vb), std::move(ib), loaded_submesh.textures } };
			submesh->aabb = loaded_submesh.aabb;
			submeshes.try_emplace(key, std::move(submesh));
		}
	}

	void Mesh::load_and_initialise_model()
	{
		mesh_name = props.path.filename().replace_extension().string();
		ModelLoader loader;
		try {
			loader = ModelLoader(make_scope<AssimpModelLoader>(props.initial_rotation), props.path, props.flags);
		} catch (const CouldNotLoadModelException& exc) {
			Log::error("Mesh", "Model could not be loaded: {}", exc.what());
			return;
		}
		load_and_initialise_model(loader.get_mesh_data());
	}

	auto Mesh::get_indices() const -> const Disarray::IndexBuffer& { return *index_buffer; }

	auto Mesh::get_vertices() const -> const Disarray::VertexBuffer& { return *vertex_buffer; }

	void Mesh::force_recreation() { load_and_initialise_model(); }

	auto Mesh::get_textures() const -> const Collections::RefVector<Disarray::Texture>& { return mesh_textures; }

	auto Mesh::get_submeshes() const -> const Collections::ScopedStringMap<Disarray::Mesh>& { return submeshes; }

	auto Mesh::has_children() const -> bool { return submeshes.size() > 0ULL; }

	struct MeshConstructor {
		auto operator()(const auto& device, MeshProperties props) const { return Mesh::construct(device, std::move(props)); }
	};

	auto Mesh::construct_deferred(const Disarray::Device& device, MeshProperties properties) -> std::future<Ref<Disarray::Mesh>>
	{
		return std::async(std::launch::async, MeshConstructor {}, std::cref(device), std::move(properties));
	}

	auto Mesh::get_aabb() const -> const AABB& { return aabb; }

	auto Mesh::invalid() const -> bool
	{
#ifdef IS_RELEASE
		return false;
#else
		return get_vertices().size() == 0 || get_indices().size() == 0;
#endif
	}

	static constexpr uint32_t s_MeshImportFlags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals
		| aiProcess_GenUVCoords |
		//		aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure
		| aiProcess_GlobalScale;

	StaticMesh::StaticMesh(const Device& dev, PipelineCache& cache, const std::filesystem::path& path)
		: device(dev)
		, file_path(path)
	{
		AssimpLogStream::initalize();

		Log::info("Mesh", "Loading mesh: {0}", file_path.string());

		importer = make_scope<Assimp::Importer, PimplDeleter<Assimp::Importer>>();
		importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		const aiScene* loaded_scene = importer->ReadFile(file_path.string(), s_MeshImportFlags);
		if (!loaded_scene) {
			Log::error("Mesh", "Failed to load mesh file: {0}", file_path.string());
			return;
		}

		scene = loaded_scene;

		if (!scene->HasMeshes()) {
			return;
		}

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		submeshes.reserve(scene->mNumMeshes);
		for (unsigned m = 0; m < scene->mNumMeshes; m++) {
			aiMesh* mesh = scene->mMeshes[m];

			StaticSubmesh& submesh = submeshes.emplace_back();
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.VertexCount = mesh->mNumVertices;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.IndexCount;

			ensure(mesh->HasPositions(), "Meshes require positions.");
			ensure(mesh->HasNormals(), "Meshes require normals.");

			// Vertices
			auto& submesh_aabb = submesh.BoundingBox;
			for (size_t i = 0; i < mesh->mNumVertices; i++) {
				ModelVertex vertex;
				vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
				vertex.normals = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
				submesh_aabb.update(vertex.pos);

				if (mesh->HasTangentsAndBitangents()) {
					vertex.tangents = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					vertex.bitangents = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}

				if (mesh->HasTextureCoords(0))
					vertex.uvs = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

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
			AABB submesh_aabb = submesh.BoundingBox;
			glm::vec3 min = glm::vec3(submesh.Transform * submesh_aabb.min_vector());
			glm::vec3 max = glm::vec3(submesh.Transform * submesh_aabb.max_vector());

			bounding_box.update(min, max);
		}

		// Materials
		const auto& whiteTexture = Disarray::Renderer::get_white_texture();
		if (scene->HasMaterials()) {
			materials.resize(scene->mNumMaterials);

			for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();
				// convert to std::string
				std::string materialName = aiMaterialName.C_Str();

				auto mi = POCMaterial::construct(device,
					{
						.shader = UnifiedShader::construct(device,
							{
								.path = FS::shader("static_mesh_combined.glsl"),
								.optimize = false,
							}),
					});
				materials[i] = mi;

				Log::info("StaticMesh", "  {0} (Index = {1})", materialName, i);
				aiString aiTexPath;
				uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
				Log::info("StaticMesh", "    TextureCount = {0}", textureCount);

				glm::vec3 albedoColor(0.8f);
				float emission = 0.0f;
				aiColor3D aiColor, aiEmission;
				if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS)
					albedoColor = { aiColor.r, aiColor.g, aiColor.b };

				if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmission) == AI_SUCCESS)
					emission = aiEmission.r;

				// TODO(edvin): Obviously
				mi->set("u_MaterialUniforms.AlbedoColor", albedoColor);
				mi->set("u_MaterialUniforms.Emission", emission);

				float shininess, metalness;
				if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
					shininess = 80.0f; // Default value

				if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
					metalness = 0.0f;

				float roughness = 1.0f - glm::sqrt(shininess / 100.0f);
				Log::info("StaticMesh", "    COLOR = {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b);
				Log::info("StaticMesh", "    ROUGHNESS = {0}", roughness);
				Log::info("StaticMesh", "    METALNESS = {0}", metalness);
				bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
				bool fallback = !hasAlbedoMap;
				if (hasAlbedoMap) {
					Ref<Texture> texture;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str())) {
						DataBuffer buffer { aiTexEmbedded->pcData, aiTexEmbedded->mWidth * aiTexEmbedded->mHeight * 4 };
						texture = Texture::construct(device,
							{
								.extent = { aiTexEmbedded->mWidth, aiTexEmbedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = buffer,
								.debug_name = aiTexPath.C_Str(),
							});
					} else {

						// TODO: Temp - this should be handled by Hazel's filesystem
						std::filesystem::path new_path = file_path;
						auto parentPath = new_path.parent_path();
						parentPath /= std::string(aiTexPath.data);
						std::string texturePath = parentPath.string();
						Log::info("StaticMesh", "    Albedo map path = {0}", texturePath);
						texture = Texture::construct(device,
							{
								.path = texturePath,
								.debug_name = aiTexPath.C_Str(),
							});
					}

					if (texture) {
						Log::info("Mesh", "Loaded albedo!");
						mi->set("u_AlbedoTexture", texture);
						mi->set("u_MaterialUniforms.AlbedoColor", glm::vec3(1.0f));
					} else {
						Log::error("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback) {
					Log::info("StaticMesh", "    No albedo map");
					mi->set("u_AlbedoTexture", whiteTexture);
				}

				// Normal maps
				bool hasNormalMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasNormalMap;
				if (hasNormalMap) {
					Ref<Texture> texture;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str())) {
						texture = Texture::construct(device,
							{
								.extent = { aiTexEmbedded->mWidth, aiTexEmbedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = { aiTexEmbedded->pcData, aiTexEmbedded->mWidth * aiTexEmbedded->mHeight * 3 },
								.debug_name = aiTexPath.C_Str(),
							});
					} else {

						// TODO: Temp - this should be handled by Hazel's filesystem
						std::filesystem::path new_path = file_path;
						auto parentPath = new_path.parent_path();
						parentPath /= std::string(aiTexPath.data);
						std::string texturePath = parentPath.string();
						Log::info("StaticMesh", "    Normal map path = {0}", texturePath);
						texture = Texture::construct(device,
							{
								.path = texturePath,
								.debug_name = aiTexPath.C_Str(),
							});
					}

					if (texture) {
						Log::info("StaticMesh", "Loaded normal map!");
						mi->set("u_NormalTexture", texture);
						mi->set("u_MaterialUniforms.UseNormalMap", true);
					} else {
						Log::error("Mesh", "    Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback) {
					Log::info("StaticMesh", "    No normal map");
					mi->set("u_NormalTexture", whiteTexture);
					mi->set("u_MaterialUniforms.UseNormalMap", false);
				}

				// Roughness map
				bool hasRoughnessMap = aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasRoughnessMap;
				if (hasRoughnessMap) {
					Ref<Texture> texture;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str())) {
						texture = Texture::construct(device,
							{
								.extent = { aiTexEmbedded->mWidth, aiTexEmbedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = { aiTexEmbedded->pcData, aiTexEmbedded->mWidth * aiTexEmbedded->mHeight * 3 },
								.debug_name = aiTexPath.C_Str(),
							});
					} else {

						// TODO: Temp - this should be handled by Hazel's filesystem
						std::filesystem::path new_path = file_path;
						auto parentPath = new_path.parent_path();
						parentPath /= std::string(aiTexPath.data);
						std::string texturePath = parentPath.string();
						Log::info("StaticMesh", "    Roughness map path = {0}", texturePath);
						texture = Texture::construct(device,
							{
								.path = texturePath,
								.debug_name = aiTexPath.C_Str(),
							});
					}

					if (texture) {
						Log::info("StaticMesh", "Loaded roughness map!");
						mi->set("u_RoughnessTexture", texture);
						mi->set("u_MaterialUniforms.Roughness", 1.0f);
					} else {
						Log::error("Mesh", "    Could not load roughness: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback) {
					Log::info("StaticMesh", "    No roughness map");
					mi->set("u_RoughnessTexture", whiteTexture);
					mi->set("u_MaterialUniforms.Roughness", roughness);
				}

				bool metalnessTextureFound = false;
				for (std::uint32_t p = 0; p < aiMaterial->mNumProperties; p++) {
					auto prop = aiMaterial->mProperties[p];

					if (prop->mType == aiPTI_String) {
						uint32_t strLength = *(uint32_t*)prop->mData;
						std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (key == "$raw.ReflectionFactor|file") {
							Ref<Texture> texture;
							if (auto aiTexEmbedded = scene->GetEmbeddedTexture(str.data())) {
								texture = Texture::construct(device,
									{
										.extent = { aiTexEmbedded->mWidth, aiTexEmbedded->mHeight },
										.format = ImageFormat::RGB,
										.data_buffer = { aiTexEmbedded->pcData, aiTexEmbedded->mWidth * aiTexEmbedded->mHeight * 3 },
										.debug_name = str,
									});
							} else {
								// TODO: Temp - this should be handled by Hazel's filesystem
								std::filesystem::path new_path = file_path;
								auto parentPath = new_path.parent_path();
								parentPath /= std::string(aiTexPath.data);
								std::string texturePath = parentPath.string();
								Log::info("StaticMesh", "    Metalnesss map path = {0}", texturePath);
								texture = Texture::construct(device,
									{
										.path = texturePath,
										.debug_name = aiTexPath.C_Str(),
									});
							}

							if (texture) {
								metalnessTextureFound = true;
								mi->set("u_MetalnessTexture", texture);
								mi->set("u_MaterialUniforms.Metalness", 1.0f);
							} else {
								Log::error("Mesh", "    Could not load texture: {0}", str);
							}
							break;
						}
					}
				}

				fallback = !metalnessTextureFound;
				if (fallback) {
					Log::info("StaticMesh", "    No metalness map");
					mi->set("u_MetalnessTexture", whiteTexture);
					mi->set("u_MaterialUniforms.Metalness", metalness);
				}
			}
			Log::info("StaticMesh", "------------------------");
		} else {
			auto mi = POCMaterial::construct(device,
				{
					.shader = UnifiedShader::construct(device,
						{
							.path = FS::shader("static_mesh_combined.glsl"),
							.optimize = false,
						}),
					.name = "Default",
				});

			mi->set("u_MaterialUniforms.AlbedoColor", glm::vec3(0.8f));
			mi->set("u_MaterialUniforms.Emission", 0.0f);
			mi->set("u_MaterialUniforms.Metalness", 0.0f);
			mi->set("u_MaterialUniforms.Roughness", 0.8f);
			mi->set("u_MaterialUniforms.UseNormalMap", false);

			mi->set("u_AlbedoTexture", whiteTexture);
			mi->set("u_MetalnessTexture", whiteTexture);
			mi->set("u_RoughnessTexture", whiteTexture);
			materials.push_back(mi);
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
		glm::mat4 to_mat4_from_assimp(const aiMatrix4x4& matrix)
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

	void StaticMesh::traverse_nodes(aiNode* node, const glm::mat4& parent_transform, std::uint32_t level)
	{
		const glm::mat4 local_transform = to_mat4_from_assimp(node->mTransformation);
		const glm::mat4 transform = parent_transform * local_transform;
		node_map[node].resize(node->mNumMeshes);
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			const std::uint32_t mesh = node->mMeshes[i];
			auto& submesh = submeshes[mesh];
			submesh.NodeName = node->mName.C_Str();
			submesh.Transform = transform;
			submesh.LocalTransform = local_transform;
			node_map[node][i] = mesh;
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			traverse_nodes(node->mChildren[i], transform, level + 1);
	}

} // namespace Vulkan

} // namespace Disarray
