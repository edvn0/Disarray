#include "DisarrayPCH.hpp"

#include <glm/glm.hpp>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>

#include <concepts>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/Ensure.hpp"
#include "core/Log.hpp"
#include "core/PointerDefinition.hpp"
#include "core/String.hpp"
#include "core/Types.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "core/filesystem/FileIO.hpp"
#include "graphics/ImageLoader.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/MeshMaterial.hpp"
#include "graphics/ModelVertex.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/StaticMesh.hpp"
#include "graphics/Texture.hpp"
#include "graphics/model_loaders/AssimpModelLoader.hpp"
#include "util/BitCast.hpp"
#include "vulkan/MeshMaterial.hpp"
#include "vulkan/SingleShader.hpp"
#include "vulkan/StaticMesh.hpp"
#include "vulkan/Texture.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

namespace Disarray {

struct InMemoryImageLoader {
	explicit InMemoryImageLoader(const void* input_data, std::integral auto input_width, std::integral auto input_height, DataBuffer& input_buffer)
		: buffer(input_buffer)
	{
		if (input_data == nullptr) {
			Log::error("InMemoryImageLoader", "Could not load embedded texture.");
			return;
		}

		constexpr std::int32_t requested_channels = STBI_rgb_alpha;
		const auto size = input_height > 0 ? input_width * input_height * requested_channels : input_width;

		stbi_set_flip_vertically_on_load(true);
		auto* loaded_image
			= stbi_load_from_memory(static_cast<const std::uint8_t*>(input_data), size, &width, &height, &channels, requested_channels);

		const auto span = std::span {
			loaded_image,
			static_cast<std::size_t>(width * height * channels),
		};
		const auto padded_buffer = pad_with_alpha(span);
		const auto loaded_size = width * height * requested_channels;

		const DataBuffer data_buffer { padded_buffer.data(), loaded_size };
		stbi_image_free(loaded_image);

		input_buffer.copy_from(data_buffer);
	}

	std::int32_t width {};
	std::int32_t height {};
	std::int32_t channels {};
	DataBuffer& buffer;

private:
	auto pad_with_alpha(const auto data) const -> std::vector<uint8_t>
	{
		if (channels == 4) {
			// Already has an alpha channel
			return { data.begin(), data.end() };
		}

		std::vector<uint8_t> new_data(width * height * 4); // 4 channels for RGBA

		for (auto y = 0; y < height; ++y) {
			for (auto x = 0; x < width; ++x) {
				const int new_index = (y * width + x) * 4;
				const int old_index = (y * width + x) * channels;

				for (int j = 0; j < channels; ++j) {
					new_data[new_index + j] = data[old_index + j];
				}

				new_data[new_index + 3] = 255;
			}
		}

		return new_data;
	}
};

struct ImporterPimpl {
	Scope<Assimp::Importer> importer { nullptr };
	std::unordered_map<aiNode*, std::vector<std::uint32_t>> node_map;
	const aiScene* scene;
};
template <> auto PimplDeleter<ImporterPimpl>::operator()(ImporterPimpl* ptr) noexcept -> void { delete ptr; }

namespace Vulkan {

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

	auto traverse_nodes(
		auto& submeshes, auto& importer, aiNode* node, const glm::mat4& parent_transform = glm::mat4 { 1.0F }, std::uint32_t level = 0) -> void
	{
		if (node == nullptr) {
			return;
		}

		const glm::mat4 local_transform = to_mat4_from_assimp(node->mTransformation);
		const glm::mat4 transform = parent_transform * local_transform;
		importer->node_map[node].resize(node->mNumMeshes);
		for (std::uint32_t i = 0; i < node->mNumMeshes; i++) {
			const std::uint32_t mesh = node->mMeshes[i];
			auto& submesh = submeshes[mesh];
			submesh.node_name = node->mName.C_Str();
			submesh.transform = transform;
			submesh.local_transform = local_transform;
			importer->node_map[node][i] = mesh;
		}

		for (std::uint32_t i = 0; i < node->mNumChildren; i++) {
			traverse_nodes(submeshes, importer, node->mChildren[i], transform, level + 1);
		}
	}

	static constexpr std::uint32_t mesh_import_flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType
		| aiProcess_GenNormals | aiProcess_FlipWindingOrder | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights
		| aiProcess_ValidateDataStructure | aiProcess_GlobalScale;

	StaticMesh::StaticMesh(const Disarray::Device& dev, const std::filesystem::path& path)
		: device(dev)
		, file_path(path)
	{
		importer = make_scope<ImporterPimpl, PimplDeleter<ImporterPimpl>>();
		importer->importer = make_scope<Assimp::Importer>();
		importer->importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		const aiScene* loaded_scene = importer->importer->ReadFile(file_path.string(), mesh_import_flags);
		if (loaded_scene == nullptr) {
			Log::error("Mesh", "Failed to load mesh file: {0}", file_path.string());
			return;
		}

		importer->scene = loaded_scene;

		if (!importer->scene->HasMeshes()) {
			return;
		}

		std::uint32_t vertex_count = 0;
		std::uint32_t index_count = 0;

		const auto num_meshes = importer->scene->mNumMeshes;

		submeshes.reserve(num_meshes);
		for (std::uint32_t submesh_index = 0; submesh_index < num_meshes; submesh_index++) {
			aiMesh* mesh = importer->scene->mMeshes[submesh_index];

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
			for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
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
			for (std::size_t i = 0; i < mesh->mNumFaces; i++) {
				ensure(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
				const Index index = {
					mesh->mFaces[i].mIndices[0],
					mesh->mFaces[i].mIndices[1],
					mesh->mFaces[i].mIndices[2],
				};
				indices.push_back(index);
			}
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

		traverse_nodes(submeshes, importer, importer->scene->mRootNode);

		for (const auto& submesh : submeshes) {
			const auto& submesh_aabb = submesh.bounding_box;
			const auto min = glm::vec3(submesh.transform * submesh_aabb.min_vector());
			const auto max = glm::vec3(submesh.transform * submesh_aabb.max_vector());

			bounding_box.update(min, max);
		}

		// Materials
		const auto& white_texture = Renderer::get_white_texture();
		const auto num_materials = importer->scene->mNumMaterials;
		const std::span materials_span { importer->scene->mMaterials, num_materials };

		if (materials_span.empty()) {
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
			submesh_material->set("pc.emission", 0.1F);
			submesh_material->set("pc.metalness", 0.1F);
			submesh_material->set("pc.roughness", default_roughness_and_albedo);
			submesh_material->set("pc.use_normal_map", false);

			submesh_material->set("albedo_map", white_texture);
			submesh_material->set("normal_map", white_texture);
			submesh_material->set("metalness_map", white_texture);
			submesh_material->set("roughness_map", white_texture);
			materials.push_back(submesh_material);

			return;
		}

		materials.resize(num_materials);

		std::size_t i = 0;
		for (const auto* ai_material : materials_span) {
			auto ai_material_name = ai_material->GetName();
			// convert to std::string
			const std::string material_name = ai_material_name.C_Str();

			auto submesh_material = MeshMaterial::construct(device,
				MeshMaterialProperties {
					SingleShader::construct(device,
						{
							.path = FS::shader("basic_combined.glsl"),
							.optimize = false,
						}),
				});
			materials[i++] = submesh_material;
			submesh_material->set("diffuse_map", white_texture);
			submesh_material->set("specular_map", white_texture);

			aiString ai_tex_path;
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

			handle_albedo_map(white_texture, ai_material, submesh_material, ai_tex_path);
			handle_normal_map(white_texture, ai_material, submesh_material, ai_tex_path);

			auto roughness = 1.0F - glm::sqrt(shininess / 100.0f);
			handle_roughness_map(white_texture, ai_material, submesh_material, ai_tex_path, roughness);
			handle_metalness_map(white_texture, ai_material, submesh_material, metalness);
		}
	}

	void StaticMesh::handle_albedo_map(const Ref<Disarray::Texture>& white_texture, const aiMaterial* ai_material,
		Ref<Disarray::MeshMaterial> submesh_material, aiString ai_tex_path) const
	{
		const bool has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_tex_path) == AI_SUCCESS;
		auto fallback = !has_albedo_map;
		if (has_albedo_map) {
			Ref<Disarray::Texture> texture;
			if (const auto* ai_texture_embedded = importer->scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
				DataBuffer buffer;
				const InMemoryImageLoader loader {
					ai_texture_embedded->pcData,
					ai_texture_embedded->mWidth,
					ai_texture_embedded->mHeight,
					buffer,
				};
				auto width = loader.width;
				auto height = loader.height;
				texture = Texture::construct(device,
						{
							.extent = { width, height, },
							.format = ImageFormat::SRGB,
							.data_buffer = buffer,
							.debug_name = std::filesystem::path { ai_tex_path.C_Str() }.filename().string(),
						});
			} else {
				texture = read_texture_from_file_path(ai_tex_path.C_Str());
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
	}

	void StaticMesh::handle_normal_map(const Ref<Disarray::Texture>& white_texture, const aiMaterial* ai_material,
		Ref<Disarray::MeshMaterial> submesh_material, aiString ai_tex_path) const
	{
		auto has_normal_map = ai_material->GetTexture(aiTextureType_NORMALS, 0, &ai_tex_path) == AI_SUCCESS;
		auto fallback = !has_normal_map;
		if (has_normal_map) {
			Ref<Disarray::Texture> texture;
			if (const auto* ai_texture_embedded = importer->scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
				texture = Texture::construct(device,
					{
						.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
						.format = ImageFormat::RGB,
						.data_buffer = { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 3 },
						.debug_name = std::filesystem::path { ai_tex_path.C_Str() }.filename().string(),
					});
			} else {
				texture = read_texture_from_file_path(ai_tex_path.C_Str());
			}

			if (texture) {
				submesh_material->set("normal_map", texture);
				submesh_material->set("pc.use_normal_map", true);
				submesh_material->get_properties().has_normal_map = true;
			} else {
				fallback = true;
			}
		}

		if (fallback) {
			submesh_material->set("normal_map", white_texture);
			submesh_material->set("pc.use_normal_map", false);
		}
	}

	void StaticMesh::handle_roughness_map(const Ref<Disarray::Texture>& white_texture, const aiMaterial* ai_material,
		Ref<Disarray::MeshMaterial> submesh_material, aiString ai_tex_path, float roughness) const
	{
		const auto has_roughness_map = ai_material->GetTexture(aiTextureType_SHININESS, 0, &ai_tex_path) == AI_SUCCESS;
		auto fallback = !has_roughness_map;
		if (has_roughness_map) {
			Ref<Disarray::Texture> texture;
			if (const auto* ai_texture_embedded = importer->scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
				texture = Texture::construct(device,
					{
						.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
						.format = ImageFormat::RGB,
						.data_buffer = { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 3 },
						.debug_name = std::filesystem::path { ai_tex_path.C_Str() }.filename().string(),
					});
			} else {
				texture = read_texture_from_file_path(ai_tex_path.C_Str());
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
	}

	void StaticMesh::handle_metalness_map(const Ref<Disarray::Texture>& white_texture, const aiMaterial* ai_material,
		Ref<Disarray::MeshMaterial> submesh_material, float metalness) const
	{
		bool has_metalness_texture = false;
		for (std::uint32_t property_index = 0; property_index < ai_material->mNumProperties; property_index++) {
			if (auto* prop = ai_material->mProperties[property_index]; prop->mType == aiPTI_String) {
				auto str_length = *bit_cast<std::uint32_t*>(prop->mData);
				std::string str(prop->mData + 4, str_length);

				if (const std::string key = prop->mKey.data; key == "$raw.ReflectionFactor|file") {
					Ref<Disarray::Texture> texture;
					if (const auto* ai_texture_embedded = importer->scene->GetEmbeddedTexture(str.data())) {
						texture = Texture::construct(device,
							{
								.extent = { ai_texture_embedded->mWidth, ai_texture_embedded->mHeight },
								.format = ImageFormat::RGB,
								.data_buffer = { ai_texture_embedded->pcData, ai_texture_embedded->mWidth * ai_texture_embedded->mHeight * 3, },
								.debug_name = str,
							});
					} else {
						texture = read_texture_from_file_path(str);
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

		auto fallback = !has_metalness_texture;
		if (fallback) {
			submesh_material->set("metalness_map", white_texture);
			submesh_material->set("pc.metalness", metalness);
		}
	}

	auto StaticMesh::read_texture_from_file_path(const std::string& texture_path) const -> Ref<Disarray::Texture>
	{
		using namespace std::filesystem;
		const auto path_to_texture = FS::texture(path { texture_path }.filename());

		const auto parent = this->file_path.parent_path();

		path constructed_path {};
		const auto parent_directory_is_either_model_or_textures_directory = parent == FS::model_directory() || parent == FS::texture_directory();
		if (FS::exists(parent) && !parent_directory_is_either_model_or_textures_directory) {
			// Well we guess that the texture is in the same directory as the mesh
			const auto requested_file_name = path_to_texture.filename();
			constructed_path = parent / requested_file_name;
		} else {
			constructed_path = path { path_to_texture };
		}

		try {
			return Texture::construct(device,
				{
					.path = constructed_path,
					.debug_name = constructed_path.filename().string(),
				});
		} catch (const ResultException& e) {
			Log::error("Mesh", "Could not load texture: {0}. Exception: {1}", path_to_texture, e.what());
			return Ref<Disarray::Texture>(nullptr);
		}
	}

} // namespace Vulkan

} // namespace Disarray
