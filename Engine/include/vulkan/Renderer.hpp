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
		std::uint32_t max_identifiers {};
	};

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
		void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::GeometryProperties&) override;
		void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const glm::mat4& transform) override;
		void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4& transform) override;
		void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties&) override;
		void submit_batched_geometry(Disarray::CommandExecutor&) override;
		void on_batch_full(std::function<void(Disarray::Renderer&)>&&) override;
		void flush_batch(Disarray::CommandExecutor&) override;
		// End IGraphics

		// IGraphicsResource
		void expose_to_shaders(Disarray::Image&) override;
		VkDescriptorSet get_descriptor_set(std::uint32_t index) override { return descriptors[index].set; }
		VkDescriptorSet get_descriptor_set() override { return get_descriptor_set(swapchain.get_current_frame()); };
		const std::vector<VkDescriptorSetLayout>& get_descriptor_set_layouts() override { return layouts; }
		// End IGraphicsResource

		void on_resize() override;
		PipelineCache& get_pipeline_cache() override { return *pipeline_cache; }

		void begin_frame(Camera&) override;
		void end_frame() override;

		void force_recreation() override;

		auto* get_push_constant() const { return &pc; }
		auto& get_editable_push_constant() { return pc; }

	private:
		void add_geometry_to_batch(Geometry, const GeometryProperties&);

		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		Scope<Disarray::PipelineCache> pipeline_cache;
		std::vector<Ref<Disarray::Texture>> texture_cache;
		Ref<Disarray::Framebuffer> geometry_framebuffer;
		BatchRenderer<max_objects> render_batch;

		std::function<void(Disarray::Renderer&)> on_batch_full_func = [](auto&) {};

		// TODO: FrameDescriptor::construct(device, props)....
		struct FrameDescriptor {
			VkDescriptorSet set;
			void destroy(Disarray::Device& dev);
		};
		std::vector<VkDescriptorSetLayout> layouts;
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
