#include "graphics/RenderPass.hpp"

#include "vulkan/RenderPass.hpp"

namespace Disarray {

	Ref<RenderPass> RenderPass::construct(Disarray::Device& device, const Disarray::RenderPassProperties& props)
	{
		return make_ref<Vulkan::RenderPass>(device, props);
	}

}