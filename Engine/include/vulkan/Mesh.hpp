#pragma once

#include "graphics/Mesh.hpp"

#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

	class Mesh : public Disarray::Mesh {
	public:
		Mesh(Ref<Disarray::Device>, Ref<Disarray::Swapchain>, Ref<Disarray::PhysicalDevice>, const MeshProperties&);
		~Mesh() override;

		Ref<Disarray::Pipeline> get_pipeline() override { return props.pipeline; }
		Ref<Disarray::IndexBuffer> get_indices() override { return indices; }
		Ref<Disarray::VertexBuffer> get_vertices() override { return vertices; }

	private:
		Ref<Disarray::Device> device;
		MeshProperties props;

		Ref<Disarray::VertexBuffer> vertices;
		Ref<Disarray::IndexBuffer> indices;
	};

} // namespace Disarray::Vulkan