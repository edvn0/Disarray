#pragma once

#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/Pipeline.hpp"

#include <array>

namespace Disarray {

	static constexpr auto max_objects = 500;

	// Forward declaration for the vulkan renderer!
	class Renderer;

	template <class T, std::size_t Objects = max_objects, std::size_t VertexCount = 0>
		requires(std::is_default_constructible_v<T> && VertexCount != 0)
	struct RenderBatchFor {
		static constexpr auto vertex_count = VertexCount;

		// We want for example to be able to write 100 quads, which requires 100 * vertex_count(Quad) = 4 vertices
		std::array<T, Objects * VertexCount> vertices {};
		Scope<Disarray::IndexBuffer> index_buffer { nullptr };
		Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };

		// From the pipeline cache!
		Ref<Vulkan::Pipeline> pipeline { nullptr };

		std::uint32_t submitted_vertices { 0 };
		std::uint32_t submitted_indices { 0 };
		std::uint32_t submitted_objects { 0 };

		void construct(Renderer&, Disarray::Device&);
		void submit(Renderer&, Disarray::CommandExecutor&);
		void create_new(const GeometryProperties&);

		void reset()
		{
			T default_construction;
			vertices.fill(default_construction);
			submitted_indices = 0;
			submitted_vertices = 0;
			submitted_objects = 0;
		}

	private:
		void prepare_data() { vertex_buffer->set_data(vertices.data(), submitted_vertices * sizeof(T)); }

		std::size_t buffer_size() { return vertex_count * max_objects * sizeof(T); }

		T& emplace() { return vertices[submitted_vertices++]; }
	};

	struct QuadVertex {
		glm::vec3 pos { 1.0f };
		glm::vec2 uvs { 1.0f };
		glm::vec2 normals { 1.0f };
		glm::vec4 colour { 1.0f };
		std::uint32_t identifier { 0 };
	};
	static constexpr auto quad_vertex_count = 4;

	struct LineVertex {
		glm::vec3 pos { 1.0f };
		glm::vec4 colour { 1.0f };
	};
	static constexpr auto line_vertex_count = 2;

	template <std::size_t Objects = max_objects> struct BatchRenderer {
	private:
		struct GeometrySubmission {
			Geometry geometry;
			std::uint32_t submitted;
		};

	public:
		RenderBatchFor<QuadVertex, Objects, quad_vertex_count> quads;
		RenderBatchFor<LineVertex, Objects, line_vertex_count> lines;

		// How many times have we submitted geometries?
		// Used by shaders to determine scale of picking count
		std::uint32_t submitted_geometries { 0 };

		void reset()
		{
			submitted_geometries = 0;
			quads.reset();
			lines.reset();
		}

		void flush()
		{
			submitted_geometries = 0;
			quads.flush();
			lines.flush();
		}

		constexpr bool is_full() const
		{
			bool batch_would_be_full = false;
			std::array<std::uint32_t, 2> batch_objects { quads.submitted_objects, lines.submitted_objects };

			for (const auto& count : batch_objects) {
				batch_would_be_full |= (count >= Objects);

				if (batch_would_be_full)
					break;
			}
			return batch_would_be_full;
		}

		void submit(Renderer&, Disarray::CommandExecutor&);
	};

} // namespace Disarray
