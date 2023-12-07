#pragma once

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "graphics/AABB.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/MeshMaterial.hpp"
#include "graphics/StaticMesh.hpp"
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
	StaticMesh(const Disarray::Device&, const std::filesystem::path& path);
	~StaticMesh() override = default;

	auto get_vertices() const -> const Disarray::VertexBuffer& override { return *vertex_buffer; }
	auto get_indices() const -> const Disarray::IndexBuffer& override { return *index_buffer; }

	auto get_submeshes() const -> const std::vector<StaticSubmesh>& override { return submeshes; }

	auto get_vertex_vector() const -> const auto& { return vertices; }
	auto get_index_vector() const -> const auto& { return indices; }

	auto get_materials() const -> const Collections::RefVector<Disarray::MeshMaterial>& override { return materials; }
	auto get_path() const -> const auto& { return file_path; }

	auto get_aabb() const -> const AABB& override { return bounding_box; }

private:
	void traverse_nodes(aiNode* node, const glm::mat4& parent_transform = glm::mat4(1.0F), std::uint32_t level = 0);

	const Disarray::Device& device;
	std::vector<StaticSubmesh> submeshes {};

	Scope<Assimp::Importer, PimplDeleter<Assimp::Importer>> importer;

	Ref<Disarray::VertexBuffer> vertex_buffer;
	Ref<Disarray::IndexBuffer> index_buffer;

	std::vector<ModelVertex> vertices;
	std::vector<Index> indices;
	std::unordered_map<aiNode*, std::vector<std::uint32_t>> node_map;
	const aiScene* scene;

	Collections::RefVector<Disarray::MeshMaterial> materials {};
	AABB bounding_box;

	std::filesystem::path file_path;
};

} // namespace Disarray::Vulkan
