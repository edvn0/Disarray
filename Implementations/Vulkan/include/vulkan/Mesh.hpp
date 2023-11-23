#pragma once

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "graphics/AABB.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Texture.hpp"
#include "graphics/VertexBuffer.hpp"

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp {
class Importer;
}

namespace Disarray::Vulkan {

class StaticMesh : public Disarray::StaticMesh {
	DISARRAY_MAKE_NONCOPYABLE(StaticMesh)
public:
	StaticMesh(const Disarray::Device&, PipelineCache& cache, const std::filesystem::path& path);
	~StaticMesh() override = default;

	auto get_vertices() const -> const Disarray::VertexBuffer& override { return *vertex_buffer; }
	auto get_indices() const -> const Disarray::IndexBuffer& override { return *index_buffer; }

	auto get_submeshes() const -> const std::vector<StaticSubmesh>& override { return submeshes; }

	auto get_vertex_vector() const -> const auto& { return vertices; }
	auto get_index_vector() const -> const auto& { return indices; }

	auto get_materials() const -> const Collections::RefVector<Disarray::POCMaterial>& override { return materials; }
	auto get_path() const -> const auto& { return file_path; }

	const AABB& get_aabb() const override { return bounding_box; }

private:
	void traverse_nodes(aiNode* node, const glm::mat4& parent_transform = glm::mat4(1.0f), std::uint32_t level = 0);

private:
	const Disarray::Device& device;
	std::vector<StaticSubmesh> submeshes {};

	Scope<Assimp::Importer, PimplDeleter<Assimp::Importer>> importer;

	Ref<Disarray::VertexBuffer> vertex_buffer;
	Ref<Disarray::IndexBuffer> index_buffer;

	std::vector<ModelVertex> vertices;
	std::vector<Index> indices;
	std::unordered_map<aiNode*, std::vector<std::uint32_t>> node_map;
	const aiScene* scene;

	Collections::RefVector<Disarray::POCMaterial> materials {};
	AABB bounding_box;

	std::filesystem::path file_path;
};

class Mesh : public Disarray::Mesh {
	DISARRAY_MAKE_NONCOPYABLE(Mesh)
public:
	Mesh(const Disarray::Device&, MeshProperties);
	Mesh(const Disarray::Device&, const Submesh&);
	~Mesh() override;

	auto get_indices() const -> const Disarray::IndexBuffer& override;
	auto get_vertices() const -> const Disarray::VertexBuffer& override;
	auto get_aabb() const -> const AABB& override;

	[[nodiscard]] auto invalid() const -> bool override;

	auto get_materials() const -> const Collections::RefVector<Disarray::Material>& override { return materials; }

	auto get_submeshes() const -> const Collections::ScopedStringMap<Disarray::Mesh>& override;
	auto get_textures() const -> const Collections::RefVector<Disarray::Texture>& override;
	auto has_children() const -> bool override;

	void force_recreation() override;

	static auto construct_deferred(const Disarray::Device&, MeshProperties) -> std::future<Ref<Disarray::Mesh>>;

private:
	void load_and_initialise_model();
	void load_and_initialise_model(const ImportedMesh&);

	Mesh(const Disarray::Device& dev, Scope<Disarray::VertexBuffer> vertices, Scope<Disarray::IndexBuffer> indices,
		const std::vector<Ref<Disarray::Texture>>& textures);

	const Disarray::Device& device;

	Collections::ScopedStringMap<Disarray::Mesh> submeshes {};

	Collections::RefVector<Disarray::Material> materials {};

	Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };
	Scope<Disarray::IndexBuffer> index_buffer { nullptr };
	Collections::RefVector<Disarray::Texture> mesh_textures;

	AABB aabb {};
	std::string mesh_name {};
};

} // namespace Disarray::Vulkan
