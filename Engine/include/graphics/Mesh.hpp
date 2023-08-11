#pragma once

#include "core/DisarrayObject.hpp"
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
		std::filesystem::path path {};
		Ref<Pipeline> pipeline { nullptr };
		glm::mat4 initial_rotation { 1.0f };
	};

	class Mesh : public ReferenceCountable {
		DISARRAY_OBJECT(Mesh)
	public:
		virtual VertexBuffer& get_vertices() = 0;
		virtual IndexBuffer& get_indices() = 0;
		virtual const VertexBuffer& get_vertices() const = 0;
		virtual const IndexBuffer& get_indices() const = 0;

		virtual const MeshProperties& get_properties() const = 0;
		virtual MeshProperties& get_properties() = 0;

		static Ref<Mesh> construct(Disarray::Device&, const MeshProperties& = {});
	};

} // namespace Disarray
