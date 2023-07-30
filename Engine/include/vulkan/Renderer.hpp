#pragma once

#include "graphics/CommandExecutor.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray::Vulkan {

	struct PushConstant {
		glm::mat4 object_transform {1.0f};
		glm::vec4 colour {1.0f};
	};

	class Renderer: public Disarray::Renderer {
	public:
		Renderer(Ref<Device>, Ref<Swapchain>, Ref<PhysicalDevice>, const RendererProperties&);
		~Renderer() override;

		void begin_pass(Ref<Disarray::CommandExecutor> command_executor, Ref<Disarray::RenderPass> render_pass, Ref<Disarray::Framebuffer> fb) override;
		void end_pass(Ref<Disarray::CommandExecutor>) override;

		void draw_mesh(Ref<Disarray::CommandExecutor>, Ref<Disarray::Mesh> mesh) override;
		void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties &) override;

		void set_extent(const Disarray::Extent &) override;
		PipelineCache & get_pipeline_cache() override { return pipeline_cache; }

		void begin_frame(UsageBadge<Disarray::App>) override {}
		void end_frame(UsageBadge<Disarray::App>) override {}

	private:
		Ref<Device> device;
		Ref<Swapchain> swapchain;
		PipelineCache pipeline_cache;
		RendererProperties props;
		Extent extent;
		PushConstant pc {};
	};

}