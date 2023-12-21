#pragma once

#include "core/ReferenceCounted.hpp"
#include "graphics/AABB.hpp"
#include "graphics/MeshMaterial.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray {

struct Index {
	std::uint32_t zero { 0 };
	std::uint32_t one { 0 };
	std::uint32_t two { 0 };
};

struct StaticSubmesh {
	std::uint32_t base_vertex;
	std::uint32_t base_index;
	std::uint32_t material_index;
	std::uint32_t index_count;
	std::uint32_t vertex_count;

	glm::mat4 transform { 1.0F };
	glm::mat4 local_transform { 1.0F };
	AABB bounding_box;

	std::string node_name {};
	std::string mesh_name {};
	bool is_rigged { false };
};

struct StaticMeshProperties {
	std::filesystem::path path {};
	glm::mat4 initial_rotation { 1.0F };
	ImportFlag flags { default_import_flags };
	std::unordered_set<VertexInput> include_inputs = { std::begin(default_vertex_inputs), std::end(default_vertex_inputs) };
};

class StaticMesh : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(StaticMesh, StaticMeshProperties)
public:
	virtual auto get_vertices() const -> const VertexBuffer& = 0;
	virtual auto get_indices() const -> const IndexBuffer& = 0;

	[[nodiscard]] virtual auto get_submeshes() const -> const std::vector<StaticSubmesh>& = 0;
	[[nodiscard]] virtual auto get_materials() const -> const Collections::RefVector<Disarray::MeshMaterial>& = 0;
	[[nodiscard]] virtual auto get_materials() -> Collections::RefVector<Disarray::MeshMaterial>& = 0;

	virtual auto get_aabb() const -> const AABB& = 0;

	static auto construct(const Disarray::Device& device, const std::filesystem::path& path) -> Ref<Disarray::StaticMesh>;
};

} // namespace Disarray
