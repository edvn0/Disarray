#pragma once

#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/RenderBatch.hpp"

#include <array>
#include <glm/glm.hpp>

namespace Disarray::Vulkan {

	struct PushConstant {
		glm::mat4 object_transform { 1.0f };
		glm::vec4 colour { 1.0f };
		std::uint32_t max_identifiers {};
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
