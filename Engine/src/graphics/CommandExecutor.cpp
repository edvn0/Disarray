#include "DisarrayPCH.hpp"

#include "graphics/CommandExecutor.hpp"

#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

	Ref<CommandExecutor> CommandExecutor::construct(
		Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::CommandExecutorProperties& props)
	{
		return make_ref<Vulkan::CommandExecutor>(device, swapchain, props);
	}

} // namespace Disarray
