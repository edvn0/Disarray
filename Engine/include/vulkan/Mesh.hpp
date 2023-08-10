#pragma once

#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

	class Mesh : public Disarray::Mesh {
	public:
		Mesh(Disarray::Device&, const MeshProperties&);
		~Mesh() override;

		Disarray::Pipeline& get_pipeline() override { return *props.pipeline; }
		Disarray::IndexBuffer& get_indices() override { return *indices; }
		Disarray::VertexBuffer& get_vertices() override { return *vertices; }
		const Disarray::Pipeline& get_pipeline() const override { return *props.pipeline; }
		const Disarray::VertexBuffer& get_vertices() const override { return *vertices; }
		const Disarray::IndexBuffer& get_indices() const override { return *indices; }

	private:
		Disarray::Device& device;
		MeshProperties props;

		Ref<Disarray::VertexBuffer> vertices;
		Ref<Disarray::IndexBuffer> indices;
	};

} // namespace Disarray::Vulkan
