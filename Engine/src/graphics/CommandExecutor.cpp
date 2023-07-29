#include "graphics/CommandExecutor.hpp"

#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

	Ref<CommandExecutor> CommandExecutor::construct(Ref<Disarray::Device> device,
		Ref<Disarray::Swapchain> swapchain, Ref<Disarray::QueueFamilyIndex> queue_family,const Disarray::CommandExecutorProperties& props)
	{
		return make_ref<Vulkan::CommandExecutor>(device, swapchain, queue_family,props);
	}

	Ref<CommandExecutor> CommandExecutor::construct_from_swapchain(Ref<Disarray::Device> device,
		Ref<Disarray::Swapchain> swapchain, Ref<Disarray::QueueFamilyIndex> queue_family, Disarray::CommandExecutorProperties props)
	{
		props.owned_by_swapchain = true;
		return make_ref<Vulkan::CommandExecutor>(device, swapchain,queue_family, props);
	}

} // namespace Disarray