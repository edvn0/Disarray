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

class Mesh : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Mesh, MeshProperties)
public:
	virtual auto get_vertices() const -> const VertexBuffer& = 0;
	virtual auto get_indices() const -> const IndexBuffer& = 0;

	[[nodiscard]] virtual auto get_materials() const -> const Collections::RefVector<Disarray::Material>& = 0;
	[[nodiscard]] virtual auto get_submeshes() const -> const Collections::ScopedStringMap<Disarray::Mesh>& = 0;
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
	uint32_t BaseVertex;
	uint32_t BaseIndex;
	uint32_t MaterialIndex;
	uint32_t IndexCount;
	uint32_t VertexCount;

	glm::mat4 Transform { 1.0f }; // World transform
	glm::mat4 LocalTransform { 1.0f };
	AABB BoundingBox;

	std::string NodeName, MeshName;
	bool IsRigged = false;
};

class StaticMesh : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(StaticMesh, MeshProperties)
public:
	virtual auto get_vertices() const -> const VertexBuffer& = 0;
	virtual auto get_indices() const -> const IndexBuffer& = 0;

	[[nodiscard]] virtual auto get_submeshes() const -> const std::vector<StaticSubmesh>& = 0;
	[[nodiscard]] virtual auto get_materials() const -> const Collections::RefVector<Disarray::POCMaterial>& = 0;

	virtual auto get_aabb() const -> const AABB& = 0;

	static auto construct(const Disarray::Device& device, PipelineCache&, const std::filesystem::path& path) -> Ref<Disarray::StaticMesh>;
};

} // namespace Disarray
