#include "vulkan/CommandExecutor.hpp"

#include "core/Types.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	CommandExecutor::CommandExecutor(Ref<Disarray::Device> dev, Ref<Disarray::PhysicalDevice> pd, Ref<Disarray::Swapchain> sc,
		Ref<Disarray::Surface> surf, const Disarray::CommandExecutorProperties& properties)
		: device(dev)
		, physical_device(pd)
		, surface(surf)
		, swapchain(sc)
		, props(properties)
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
		QueueFamilyIndex index(physical_device, surface);

		VkCommandPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = index.get_graphics_family();

		verify(vkCreateCommandPool(supply_cast<Vulkan::Device>(device), &pool_info, nullptr, &command_pool));

		auto count = props.count ? *props.count : swapchain->image_count();
		if (count > Config::max_frames_in_flight)
			count = Config::max_frames_in_flight;

		command_buffers.resize(count);
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = props.is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		alloc_info.commandBufferCount = count;
		verify(vkAllocateCommandBuffers(vk_device, &alloc_info, command_buffers.data()));

		graphics_queue = cast_to<Vulkan::Device>(device)->get_graphics_queue();

		fences.resize(count);

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < count; i++) {
			verify(vkCreateFence(vk_device, &fence_create_info, nullptr, &fences[i]));
		}
	}

	void CommandExecutor::begin()
	{
		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		if (props.owned_by_swapchain) {
			active = cast_to<Vulkan::Swapchain>(swapchain)->get_drawbuffer();
		} else {
			current = swapchain->get_image_index();
			active = command_buffers[current];
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

		const auto& [image_available, render_finished] = cast_to<Vulkan::Swapchain>(swapchain)->get_presenting_semaphores();

		VkSemaphore wait_semaphores[] = { image_available };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;

		VkSemaphore signal_semaphores[] = { render_finished };
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &active;

		verify(vkWaitForFences(supply_cast<Vulkan::Device>(device), 1, &fences[current], VK_TRUE, UINT64_MAX));
		verify(vkResetFences(supply_cast<Vulkan::Device>(device), 1, &fences[current]));

		verify(vkQueueSubmit(graphics_queue, 1, &submit_info, fences[current]));
	}

	CommandExecutor::~CommandExecutor()
	{
		if (props.owned_by_swapchain)
			return;
		vkDestroyCommandPool(supply_cast<Vulkan::Device>(device), command_pool, nullptr);
	}

} // namespace Disarray::Vulkan