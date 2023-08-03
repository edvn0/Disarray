#pragma once

#include "PropertySupplier.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

	class Pipeline : public Disarray::Pipeline, public PropertySupplier<VkPipeline> {
	public:
		Pipeline(Disarray::Device&, Disarray::Swapchain&, const PipelineProperties&);
		~Pipeline() override;

		void force_recreation() override { recreate(true); };
		void recreate(bool should_clean) override;

		VkPipeline supply() const override { return pipeline; }
		VkPipelineLayout get_layout() const { return layout; }

		Disarray::Framebuffer& get_framebuffer() override;
		Disarray::RenderPass& get_render_pass() override;

	private:
		void construct_layout();
		std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo> retrieve_shader_stages(
			Ref<Disarray::Shader> vertex, Ref<Disarray::Shader> fragment) const;
		void recreate_pipeline(bool should_clean);

		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		PipelineProperties props;
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

} // namespace Disarray::Vulkan
