#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <future>

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/AABB.hpp"
#include "graphics/Device.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/VertexBuffer.hpp"

namespace Disarray {

struct MeshProperties {
	std::filesystem::path path {};
	glm::mat4 initial_rotation { 1.0F };
	ImportFlag flags { default_import_flags };
	std::unordered_set<VertexInput> include_inputs = { std::begin(default_vertex_inputs), std::end(default_vertex_inputs) };
};

struct MeshSubstructure {
	Scope<Disarray::VertexBuffer> vertices {};
	Scope<Disarray::IndexBuffer> indices {};
	std::unordered_set<std::int32_t> texture_indices {};
};

class Mesh : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Mesh, MeshProperties)
public:
	virtual auto get_vertices() const -> VertexBuffer& = 0;
	virtual auto get_indices() const -> IndexBuffer& = 0;

	[[nodiscard]] virtual auto get_submeshes() const -> const Collections::ScopedStringMap<Disarray::MeshSubstructure>& = 0;
	[[nodiscard]] virtual auto get_textures() const -> const Collections::RefVector<Disarray::Texture>& = 0;

	virtual auto get_aabb() const -> const AABB& = 0;
	virtual auto has_children() const -> bool = 0;

	[[nodiscard]] virtual auto invalid() const -> bool = 0;

	static auto construct_deferred(const Device&, MeshProperties) -> std::future<Ref<Mesh>>;
};

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
	bool is_rigged = false;
};

class StaticMesh : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(StaticMesh, MeshProperties)
public:
	virtual auto get_vertices() const -> const VertexBuffer& = 0;
	virtual auto get_indices() const -> const IndexBuffer& = 0;

	[[nodiscard]] virtual auto get_submeshes() const -> const std::vector<StaticSubmesh>& = 0;
	[[nodiscard]] virtual auto get_materials() const -> const Collections::RefVector<Disarray::MeshMaterial>& = 0;

	virtual auto get_aabb() const -> const AABB& = 0;

	static auto construct(const Disarray::Device& device, const std::filesystem::path& path) -> Ref<Disarray::StaticMesh>;
};

} // namespace Disarray
