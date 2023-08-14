#pragma once

#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

class Mesh : public Disarray::Mesh {
public:
	Mesh(const Disarray::Device&, const MeshProperties&);
	~Mesh() override;

	Disarray::IndexBuffer& get_indices() override { return *indices; }
	Disarray::VertexBuffer& get_vertices() override { return *vertices; }
	const Disarray::VertexBuffer& get_vertices() const override { return *vertices; }
	const Disarray::IndexBuffer& get_indices() const override { return *indices; }

	const MeshProperties& get_properties() const override { return props; }
	MeshProperties& get_properties() override { return props; }

private:
	const Disarray::Device& device;
	MeshProperties props;

	Ref<Disarray::VertexBuffer> vertices;
	Ref<Disarray::IndexBuffer> indices;
};

} // namespace Disarray::Vulkan
