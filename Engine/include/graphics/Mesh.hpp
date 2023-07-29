#pragma once

#include "core/Types.hpp"
#include <filesystem>

namespace Disarray {

	class Device;
	class VertexBuffer;
	class IndexBuffer;
	class Pipeline;
	class Swapchain;
	class PhysicalDevice;

	struct MeshProperties {
		std::string path;
		Ref<Pipeline> pipeline;
	};

	class Mesh {
	public:
		virtual ~Mesh() = default;

		virtual Ref<Pipeline> get_pipeline() = 0;
		virtual Ref<VertexBuffer> get_vertices() = 0;
		virtual Ref<IndexBuffer> get_indices() = 0;

		static Ref<Mesh> construct(Ref<Disarray::Device>, Ref<Disarray::Swapchain>, Ref<Disarray::PhysicalDevice>, const MeshProperties& = {});
	};

}
