#pragma once

#include "graphics/CommandExecutor.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray::Vulkan {

	class Renderer: public Disarray::Renderer {
	public:
		Renderer(Ref<Device>, Ref<Swapchain>, const RendererProperties&);
		~Renderer() override;

		void begin_pass(Ref<Disarray::CommandExecutor> command_executor, Ref<Disarray::RenderPass> render_pass, Ref<Disarray::Pipeline> pipeline, Ref<Disarray::Framebuffer> fb) override;
		void end_pass(Ref<Disarray::CommandExecutor>) override;

		void draw_mesh(Ref<Disarray::CommandExecutor>, Ref<Disarray::Mesh> mesh) override;
		void draw_planar_geometry(Ref<Disarray::CommandExecutor>, Disarray::Geometry, const Disarray::GeometryProperties &) override;

		void set_extent(const Disarray::Extent &) override;
		PipelineCache & get_pipeline_cache() override { return pipeline_cache; }

	private:
		Ref<Device> device;
		Ref<Swapchain> swapchain;
		PipelineCache pipeline_cache;
		RendererProperties props;
		Extent extent;
	};

}