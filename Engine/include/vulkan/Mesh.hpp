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

	private:
		Disarray::Device& device;
		MeshProperties props;

		Ref<Disarray::VertexBuffer> vertices;
		Ref<Disarray::IndexBuffer> indices;
	};

} // namespace Disarray::Vulkan
