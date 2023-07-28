#pragma once

#include "PropertySupplier.hpp"
#include "graphics/RenderPass.hpp"

namespace Disarray::Vulkan {

	class RenderPass: public Disarray::RenderPass, public PropertySupplier<VkRenderPass>
	{
	public:
		RenderPass(Ref<Disarray::Device>, const RenderPassProperties&);
		~RenderPass() override;

		VkRenderPass supply() const override { return render_pass; }

		void force_recreation() override { recreate(); };

	private:
		void recreate(bool should_clean = true);

		Ref<Disarray::Device> device;
		RenderPassProperties props;
		VkRenderPass render_pass;
	};
}