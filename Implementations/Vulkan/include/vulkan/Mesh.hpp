#pragma once

#include "core/Collections.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "graphics/AABB.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/ModelLoader.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Texture.hpp"
#include "graphics/VertexBuffer.hpp"

namespace Disarray::Vulkan {

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

	Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };
	Scope<Disarray::IndexBuffer> index_buffer { nullptr };
	Collections::RefVector<Disarray::Texture> mesh_textures;

	AABB aabb {};
	std::string mesh_name {};
};

} // namespace Disarray::Vulkan
