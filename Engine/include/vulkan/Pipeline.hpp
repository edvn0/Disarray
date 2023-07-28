#pragma once

#include "PropertySupplier.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

	class Pipeline : public Disarray::Pipeline, public PropertySupplier<VkPipeline> {
	public:
		Pipeline(Ref<Disarray::Device>, Ref<Disarray::Swapchain>, const PipelineProperties&);
		~Pipeline() override;

		void force_recreation() override { recreate(); };

		VkPipeline supply() const override { return pipeline; }
		VkPipelineLayout get_layout() const { return layout; }

	private:
		void construct_layout();
		std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo> retrieve_shader_stages(Ref<Shader> vertex, Ref<Shader> fragment) const;

		void recreate(bool should_clean = true);

		Ref<Disarray::Device> device;
		Ref<Disarray::Swapchain> swapchain;
		PipelineProperties props;
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

} // namespace Disarray::Vulkan