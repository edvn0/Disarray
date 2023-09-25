#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "PropertySupplier.hpp"
#include "graphics/Pipeline.hpp"

namespace Disarray::Vulkan {

class Pipeline : public Disarray::Pipeline, public PropertySupplier<VkPipeline> {
	DISARRAY_MAKE_NONCOPYABLE(Pipeline)
public:
	Pipeline(const Disarray::Device&, PipelineProperties);
	~Pipeline() override;

	auto recreate(bool /*unused*/, const Extent& /*unused*/) -> void override;

	auto force_recreation() -> void override { recreate(true, {}); };

	auto supply() const -> VkPipeline override { return pipeline; }
	auto get_layout() const -> VkPipelineLayout { return layout; }

	auto get_framebuffer() -> Disarray::Framebuffer& override;
	auto get_render_pass() -> Disarray::RenderPass& override;

private:
	void construct_layout(const Extent&);
	void try_find_or_recreate_cache();
	void recreate_pipeline(bool, const Extent&);
	auto initialise_blend_states() -> std::vector<VkPipelineColorBlendAttachmentState>;
	void register_framebuffer_callback();

	static auto retrieve_shader_stages(Ref<Disarray::Shader> vertex, Ref<Disarray::Shader> fragment)
		-> std::pair<VkPipelineShaderStageCreateInfo, VkPipelineShaderStageCreateInfo>;

	const Disarray::Device& device;
	VkPipeline pipeline { nullptr };
	VkPipelineCache cache { nullptr };
	VkPipelineLayout layout { nullptr };
};

} // namespace Disarray::Vulkan
