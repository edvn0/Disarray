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

		virtual Pipeline& get_pipeline() = 0;
		virtual VertexBuffer& get_vertices() = 0;
		virtual IndexBuffer& get_indices() = 0;

		static Ref<Mesh> construct(Disarray::Device&, Disarray::Swapchain&, const MeshProperties& = {});
	};

}
