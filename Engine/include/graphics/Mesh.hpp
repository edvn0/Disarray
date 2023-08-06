#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/VertexBuffer.hpp"

#include <filesystem>
#include <glm/glm.hpp>

namespace Disarray {

	struct MeshProperties {
		std::string path;
		Ref<Pipeline> pipeline;
		glm::mat4 initial_rotation { 1.0f };
	};

	class Mesh : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(Mesh)
	public:
		virtual Pipeline& get_pipeline() = 0;
		virtual VertexBuffer& get_vertices() = 0;
		virtual IndexBuffer& get_indices() = 0;

		static Ref<Mesh> construct(Disarray::Device&, Disarray::Swapchain&, const MeshProperties& = {});
	};

} // namespace Disarray
