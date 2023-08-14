#include "DisarrayPCH.hpp"

#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Swapchain.hpp"

namespace Disarray {

Scope<Swapchain> Swapchain::construct(Disarray::Window& window, Disarray::Device& device, Disarray::Swapchain* old)
{
	return make_scope<Vulkan::Swapchain>(window, device, old);
}

} // namespace Disarray
