#include "DisarrayPCH.hpp"

#include "graphics/CommandExecutor.hpp"

#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

	Ref<CommandExecutor> CommandExecutor::construct(
		Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::CommandExecutorProperties& props)
	{
		return make_ref<Vulkan::CommandExecutor>(device, swapchain, props);
	}

	Ref<CommandExecutor> CommandExecutor::construct(
		const Disarray::Device& device, const Disarray::Swapchain& swapchain, const Disarray::CommandExecutorProperties& props)
	{
		return make_ref<Vulkan::CommandExecutor>(device, swapchain, props);
	}

	Scope<CommandExecutor> IndependentCommandExecutor::construct(Disarray::Device& device, const Disarray::CommandExecutorProperties& props)
	{
		return make_scope<Vulkan::IndependentCommandExecutor>(device, props);
	}

} // namespace Disarray
