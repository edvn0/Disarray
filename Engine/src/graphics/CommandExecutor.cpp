#include "DisarrayPCH.hpp"

#include "graphics/CommandExecutor.hpp"

#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

auto CommandExecutor::construct(const Disarray::Device& device, const Disarray::Swapchain* swapchain, Disarray::CommandExecutorProperties props)
	-> Ref<Disarray::CommandExecutor>
{
	return make_ref<Vulkan::CommandExecutor>(device, swapchain, props);
}

auto CommandExecutor::construct_scoped(const Disarray::Device& device, Disarray::CommandExecutorProperties properties)
	-> Scope<Disarray::CommandExecutor>
{
	return make_scope<Vulkan::CommandExecutor>(device, nullptr, properties);
}

} // namespace Disarray
