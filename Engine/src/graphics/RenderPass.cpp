#include "DisarrayPCH.hpp"

#include "graphics/RenderPass.hpp"

#include "vulkan/RenderPass.hpp"

namespace Disarray {

auto RenderPass::construct(const Disarray::Device& device, Disarray::RenderPassProperties properties) -> Ref<Disarray::RenderPass>
{
	return make_ref<Vulkan::RenderPass>(device, std::move(properties));
}

} // namespace Disarray
