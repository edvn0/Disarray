#pragma once

#include "Pipeline.hpp"
#include "glm/fwd.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/VertexBuffer.hpp"

#include <array>

namespace Disarray::Vulkan {

	template <class T> constexpr std::size_t vertex_count { 0 };

	struct PushConstant {
		glm::mat4 object_transform { 1.0f };
		glm::vec4 colour { 1.0f };
	};

	static constexpr auto max_vertices = 600;

	// Forward declaration for the vulkan renderer!
	class Renderer;

	template <class T, std::size_t Vertices = max_vertices, std::size_t VertexCount = vertex_count<T>>
		requires(std::is_default_constructible_v<T> && vertex_count<T> != 0)
	struct RenderBatchFor {
		static constexpr auto vertex_count = VertexCount;

		std::array<T, Vertices * VertexCount> vertices {};
		Ref<Disarray::IndexBuffer> index_buffer { nullptr };
		Ref<Disarray::VertexBuffer> vertex_buffer { nullptr };
		Ref<Vulkan::Pipeline> pipeline { nullptr };
		std::uint32_t submitted_ts { 0 };
		std::uint32_t submitted_indices { 0 };

		void construct(Renderer&, Disarray::Device&, Disarray::Swapchain&);
		void submit(Renderer&, Disarray::CommandExecutor&);
		void create_new(const GeometryProperties&);

		void reset()
		{
			T default_construction;
			vertices.fill(default_construction);
			submitted_indices = 0;
			submitted_ts = 0;
		}

	private:
		void prepare_data() { vertex_buffer->set_data(vertices.data(), submitted_ts * sizeof(T)); }

		T& emplace() { return vertices[submitted_ts++]; }
	};

	struct QuadVertex {
		glm::vec3 pos { 1.0f };
		glm::vec2 uvs { 1.0f };
		glm::vec2 normals { 1.0f };
		glm::vec4 colour { 1.0f };
	};
	template <> constexpr auto vertex_count<QuadVertex> = 4;

	struct LineVertex {
		glm::vec3 pos { 1.0f };
		glm::vec4 colour { 1.0f };
	};
	template <> constexpr auto vertex_count<LineVertex> = 2;

	template <std::size_t Vertices = max_vertices> struct RenderBatch {
		RenderBatchFor<QuadVertex, Vertices> quads;
		RenderBatchFor<LineVertex, Vertices> lines;

		void reset()
		{
			quads.reset();
			lines.reset();
		}

		void submit(Renderer&, Disarray::CommandExecutor&);
	};

	class Renderer : public Disarray::Renderer {
	public:
		Renderer(Device&, Swapchain&, const RendererProperties&);
		~Renderer() override;

		void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) override;
		void begin_pass(Disarray::CommandExecutor& command_executor) override { begin_pass(command_executor, *default_framebuffer); }
		void end_pass(Disarray::CommandExecutor&) override;

		void draw_mesh(Disarray::CommandExecutor&, Disarray::Mesh& mesh) override;
		void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties&) override;
		void submit_batched_geometry(Disarray::CommandExecutor&) override;

		void set_extent(const Disarray::Extent&) override;
		PipelineCache& get_pipeline_cache() override { return *pipeline_cache; }

		void begin_frame(UsageBadge<App>) override;
		void end_frame(UsageBadge<App>) override;

		void force_recreation() override;

		auto* get_push_constant() const { return &pc; }

	private:
		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		Scope<Disarray::PipelineCache> pipeline_cache;
		Ref<Disarray::Framebuffer> default_framebuffer;
		RenderBatch<max_vertices> render_batch;
		RendererProperties props;
		Extent extent;
		PushConstant pc {};
	};

} // namespace Disarray::Vulkan
