#include "graphics/Swapchain.hpp"

#include "core/Types.hpp"
#include "core/Window.hpp"
#include "vulkan/Swapchain.hpp"

namespace Disarray {

	Ref<Swapchain> Swapchain::construct(Disarray::Window& window, Disarray::Device& device, Disarray::Swapchain* old)
	{
		return make_ref<Vulkan::Swapchain>(window, device, old);
	}

} // namespace Disarray