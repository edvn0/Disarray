#pragma once

#include "PropertySupplier.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

class Pipeline : public Disarray::Pipeline, public PropertySupplier<VkPipeline> {
public:
	Pipeline(const Disarray::Device&, const PipelineProperties&);
	~Pipeline() override;

	void force_recreation() override { recreate(true); };
	void recreate(bool should_clean) override;

	VkPipeline supply() const override { return pipeline; }
	VkPipelineLayout get_layout() const { return layout; }

	Disarray::Framebuffer& get_framebuffer() override;
	Disarray::RenderPass& get_render_pass() override;

	const PipelineProperties& get_properties() const override { return props; }
	PipelineProperties& get_properties() override { return props; }

private:
	void construct_layout();
	void try_find_or_recreate_cache();
	std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo> retrieve_shader_stages(
		Ref<Disarray::Shader> vertex, Ref<Disarray::Shader> fragment) const;
	void recreate_pipeline(bool should_clean);

	const Disarray::Device& device;
	PipelineProperties props;
	VkPipeline pipeline { nullptr };
	VkPipelineCache cache { nullptr };
	VkPipelineLayout layout { nullptr };
};

} // namespace Disarray::Vulkan
