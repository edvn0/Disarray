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
#include <glm/glm.hpp>

namespace Disarray::Vulkan {

	struct PushConstant {
		glm::mat4 object_transform { 1.0f };
		glm::vec4 colour { 1.0f };
	};

	static constexpr auto max_vertices = 600;

	// Forward declaration for the vulkan renderer!
	class Renderer;

	template <class T, std::size_t Vertices = max_vertices, std::size_t VertexCount = 0>
		requires(std::is_default_constructible_v<T> && VertexCount != 0)
	struct RenderBatchFor {
		static constexpr auto vertex_count = VertexCount;

		// We want for example to be able to write 100 quads, which requires 100 * vertex_count(Quad) = 6 vertices
		std::array<T, Vertices * VertexCount> vertices {};
		Scope<Disarray::IndexBuffer> index_buffer { nullptr };
		Scope<Disarray::VertexBuffer> vertex_buffer { nullptr };

		// From the pipeline cache!
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
	static constexpr auto quad_vertex_count = 4;

	struct LineVertex {
		glm::vec3 pos { 1.0f };
		glm::vec4 colour { 1.0f };
	};
	static constexpr auto line_vertex_count = 2;

	template <std::size_t Vertices = max_vertices> struct BatchRenderer {
		RenderBatchFor<QuadVertex, Vertices, quad_vertex_count> quads;
		RenderBatchFor<LineVertex, Vertices, line_vertex_count> lines;

		void reset()
		{
			quads.reset();
			lines.reset();
		}

		void submit(Renderer&, Disarray::CommandExecutor&);
	};

	struct UBO {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 view_projection;
	};

	class Renderer : public Disarray::Renderer {
	public:
		Renderer(Device&, Swapchain&, const RendererProperties&);
		~Renderer() override;

		void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) override;
		void begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& fb) override { begin_pass(executor, fb, false); }
		void begin_pass(Disarray::CommandExecutor& command_executor) override { begin_pass(command_executor, *geometry_framebuffer); }
		void end_pass(Disarray::CommandExecutor&) override;

		// IGraphics
		void draw_mesh(Disarray::CommandExecutor&, Disarray::Mesh&, const Disarray::GeometryProperties&) override;
		void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties&) override;
		void submit_batched_geometry(Disarray::CommandExecutor&) override;
		// End IGraphics

		// IGraphicsResource
		void expose_to_shaders(Disarray::Image&) override;
		VkDescriptorSet get_descriptor_set(std::uint32_t) override;
		VkDescriptorSet get_descriptor_set() override { return get_descriptor_set(swapchain.get_current_frame()); };
		VkDescriptorSetLayout get_descriptor_set_layout() override { return layout; }
		std::uint32_t get_descriptor_set_layout_count() override { return 1; }
		// End IGraphicsResource

		void on_resize() override;
		PipelineCache& get_pipeline_cache() override { return *pipeline_cache; }

		void begin_frame(UsageBadge<App>) override;
		void end_frame(UsageBadge<App>) override;

		void force_recreation() override;

		auto* get_push_constant() const { return &pc; }

	private:
		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		Scope<Disarray::PipelineCache> pipeline_cache;
		Ref<Disarray::Framebuffer> geometry_framebuffer;
		BatchRenderer<max_vertices> render_batch;

		// TODO: FrameDescriptor::construct(device, props)....
		struct FrameDescriptor {
			VkDescriptorSet set;
			void destroy(Disarray::Device& dev);
		};
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;
		std::vector<FrameDescriptor> descriptors;
		void initialise_descriptors();

		UBO uniform {};
		std::vector<Ref<UniformBuffer>> frame_ubos;

		RendererProperties props;
		Extent extent;
		PushConstant pc {};
	};

} // namespace Disarray::Vulkan
