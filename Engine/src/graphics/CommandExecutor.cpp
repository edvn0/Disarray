#include "graphics/CommandExecutor.hpp"

#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

	Ref<CommandExecutor> CommandExecutor::construct(Ref<Disarray::Device> device, Ref<Disarray::PhysicalDevice> physical_device,
		Ref<Disarray::Swapchain> swapchain, Ref<Disarray::Surface> surface, const Disarray::CommandExecutorProperties& props)
	{
		return make_ref<Vulkan::CommandExecutor>(device, physical_device, swapchain, surface, props);
	}

	Ref<CommandExecutor> CommandExecutor::construct_from_swapchain(Ref<Disarray::Device> device, Ref<Disarray::PhysicalDevice> physical_device,
		Ref<Disarray::Swapchain> swapchain, Ref<Disarray::Surface> surface, Disarray::CommandExecutorProperties props)
	{
		props.owned_by_swapchain = true;
		return make_ref<Vulkan::CommandExecutor>(device, physical_device, swapchain, surface, props);
	}

} // namespace Disarray