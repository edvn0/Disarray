#pragma once

#include "PropertySupplier.hpp"
#include "graphics/RenderPass.hpp"

namespace Disarray::Vulkan {

	class RenderPass : public Disarray::RenderPass, public PropertySupplier<VkRenderPass> {
	public:
		RenderPass(Disarray::Device&, const RenderPassProperties&);
		~RenderPass() override;

		VkRenderPass supply() const override { return render_pass; }

		void recreate(bool should_clean, const Extent& extent) override { recreate_renderpass(should_clean); }
		void force_recreation() override { recreate_renderpass(); };

	private:
		void recreate_renderpass(bool should_clean = true);

		Disarray::Device& device;
		RenderPassProperties props;
		VkRenderPass render_pass;
	};
} // namespace Disarray::Vulkan
