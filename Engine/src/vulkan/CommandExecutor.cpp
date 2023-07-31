#include "vulkan/CommandExecutor.hpp"

#include "core/Log.hpp"
#include "core/Types.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	CommandExecutor::CommandExecutor(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::CommandExecutorProperties& properties)
		: device(dev)
		, indexes(device.get_physical_device().get_queue_family_indexes())
		, swapchain(sc)
		, props(properties)
		, is_frame_dependent_executor(properties.count.has_value() && *properties.count > 1 && !properties.owned_by_swapchain)
	{
		recreate(false);
	}

	void CommandExecutor::recreate(bool should_clean)
	{
		if (props.owned_by_swapchain)
			return;

		if (should_clean) {
			vkDestroyCommandPool(supply_cast<Vulkan::Device>(device), command_pool, nullptr);
		}

		const auto vk_device = supply_cast<Vulkan::Device>(device);

		VkCommandPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = indexes.get_graphics_family();

		verify(vkCreateCommandPool(supply_cast<Vulkan::Device>(device), &pool_info, nullptr, &command_pool));

		auto count = props.count ? *props.count : swapchain.image_count();
		if (count > Config::max_frames_in_flight)
			count = Config::max_frames_in_flight;

		command_buffers.resize(count);
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = props.is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		alloc_info.commandBufferCount = count;
		verify(vkAllocateCommandBuffers(vk_device, &alloc_info, command_buffers.data()));

		graphics_queue = cast_to<Vulkan::Device>(device).get_graphics_queue();

		fences.resize(count);

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < count; i++) {
			verify(vkCreateFence(vk_device, &fence_create_info, nullptr, &fences[i]));
		}

		image_count = count;
	}

	void CommandExecutor::begin()
	{
		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		if (props.owned_by_swapchain) {
			active = cast_to<Vulkan::Swapchain>(swapchain).get_drawbuffer();
		} else {
			active = command_buffers[buffer_index()];
		}
		verify(vkBeginCommandBuffer(active, &begin_info));
	}

	void CommandExecutor::end() { vkEndCommandBuffer(active); }

	void CommandExecutor::submit_and_end()
	{
		end();

		if (props.owned_by_swapchain) {
			active = nullptr;
			return;
		}

		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &active;
		verify(vkWaitForFences(supply_cast<Vulkan::Device>(device), 1, &fences[buffer_index()], VK_TRUE, UINT64_MAX));
		verify(vkResetFences(supply_cast<Vulkan::Device>(device), 1, &fences[buffer_index()]));

		verify(vkQueueSubmit(graphics_queue, 1, &submit_info, fences[buffer_index()]));

		verify(vkWaitForFences(supply_cast<Vulkan::Device>(device), 1, &fences[buffer_index()], VK_TRUE, UINT64_MAX));

		current = (current + 1) % image_count;
	}

	CommandExecutor::~CommandExecutor()
	{
		if (props.owned_by_swapchain)
			return;
		vkDestroyCommandPool(supply_cast<Vulkan::Device>(device), command_pool, nullptr);
		for (auto& fence : fences)
			vkDestroyFence(supply_cast<Vulkan::Device>(device), fence, nullptr);
	}

	void CommandExecutor::begin(VkCommandBufferBeginInfo begin_info)
	{
		if (props.owned_by_swapchain) {
			active = static_cast<Vulkan::Swapchain&>(swapchain).get_drawbuffer();
		} else {
			active = command_buffers[buffer_index()];
		}
		verify(vkBeginCommandBuffer(active, &begin_info));
	}

} // namespace Disarray::Vulkan