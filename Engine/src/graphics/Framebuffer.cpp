#include "DisarrayPCH.hpp"

#include "graphics/Framebuffer.hpp"

#include "core/Ensure.hpp"
#include "graphics/RenderPass.hpp"
#include "vulkan/Framebuffer.hpp"

namespace Disarray {

auto Framebuffer::construct(const Disarray::Device& device, Disarray::FramebufferProperties props) -> Ref<Disarray::Framebuffer>
{
	ensure(props.extent != Extent { 0, 0 });
	return make_ref<Vulkan::Framebuffer>(device, std::move(props));
}

} // namespace Disarray
