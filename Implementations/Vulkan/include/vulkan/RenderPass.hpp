#pragma once

#include <vulkan/vulkan.h>

#include "PropertySupplier.hpp"
#include "core/DisarrayObject.hpp"
#include "graphics/RenderPass.hpp"

namespace Disarray::Vulkan {

class RenderPass : public Disarray::RenderPass, public PropertySupplier<VkRenderPass> {
	DISARRAY_MAKE_NONCOPYABLE(RenderPass)
public:
	RenderPass(const Disarray::Device&, RenderPassProperties);
	~RenderPass() override;

	void create_with(VkRenderPassCreateInfo);

	VkRenderPass supply() const override { return render_pass; }

	void recreate(bool should_clean, const Extent& extent) override { recreate_renderpass(should_clean); }
	void force_recreation() override { recreate_renderpass(); };

private:
	void recreate_renderpass(bool should_clean = true);

	const Disarray::Device& device;
	RenderPassProperties props;
	VkRenderPass render_pass;
	VkRenderPassCreateInfo render_pass_info;
};
} // namespace Disarray::Vulkan
