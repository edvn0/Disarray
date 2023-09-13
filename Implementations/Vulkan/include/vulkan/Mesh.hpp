#pragma once

#include "core/DisarrayObject.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

class Mesh : public Disarray::Mesh {
	DISARRAY_MAKE_NONCOPYABLE(Mesh)
public:
	Mesh(const Disarray::Device&, MeshProperties);
	~Mesh() override;

	auto get_indices() const -> Disarray::IndexBuffer& override { return *indices; }
	auto get_vertices() const -> Disarray::VertexBuffer& override { return *vertices; }

	auto get_submeshes() const -> std::vector<Scope<Disarray::Mesh>> override;
	auto has_children() const -> bool override;

	void force_recreation() override;

private:
	void load_and_initialise_model();

	const Disarray::Device& device;

	Scope<Disarray::VertexBuffer> vertices;
	Scope<Disarray::IndexBuffer> indices;
};

} // namespace Disarray::Vulkan
