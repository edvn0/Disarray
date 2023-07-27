#include "graphics/Swapchain.hpp"
#include "core/Types.hpp"

#include "core/Window.hpp"
#include "vulkan/Swapchain.hpp"

namespace Disarray {

	Ref<Swapchain> Swapchain::construct(Scope<Disarray::Window>& window, Ref<Disarray::Device> device, Ref<Disarray::PhysicalDevice> physical_device)
	{
		return make_ref<Vulkan::Swapchain>(window, device, physical_device);
	}

}