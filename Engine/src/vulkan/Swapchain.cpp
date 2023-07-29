#include "vulkan/Swapchain.hpp"

#include "core/CleanupAwaiter.hpp"
#include "core/Log.hpp"
#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/SwapchainUtilities.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	Swapchain::Swapchain(
		Scope<Disarray::Window>& win, Ref<Disarray::Device> dev, Ref<Disarray::PhysicalDevice> pd, Ref<Disarray::Swapchain> old)
		: device(dev), window(win), physical_device(pd)
	{
		recreate_swapchain(old, false);
		swapchain_needs_recreation = false;

		present_queue = cast_to<Vulkan::Device>(device)->get_present_queue();
		graphics_queue = cast_to<Vulkan::Device>(device)->get_graphics_queue();


		Log::debug("Swapchain image views retrieved!");
	}

	Swapchain::~Swapchain()
	{
		cleanup_swapchain();
		Log::debug("Swapchain destroyed!");
	}

	void Swapchain::create_synchronisation_objects()
	{
		image_available_semaphores.resize(Config::max_frames_in_flight);
		render_finished_semaphores.resize(Config::max_frames_in_flight);
		in_flight_fences.resize(Config::max_frames_in_flight);

		VkSemaphoreCreateInfo semaphore_info {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		const auto vk_device = supply_cast<Vulkan::Device>(device);

		for (size_t i = 0; i < Vulkan::Config::max_frames_in_flight; i++) {
			verify(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &image_available_semaphores[i]));
			verify(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &render_finished_semaphores[i]));
			verify(vkCreateFence(vk_device, &fence_create_info, nullptr, &in_flight_fences[i]));
		}
	}

	bool Swapchain::prepare_frame()
	{
		auto vk_device = supply_cast<Vulkan::Device>(device);

		vkWaitForFences(vk_device, 1, &in_flight_fences[get_current_frame()], VK_TRUE, UINT64_MAX);

		auto result = vkAcquireNextImageKHR(vk_device, swapchain, UINT64_MAX, image_available_semaphores[get_current_frame()], VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain();
			return false;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(vk_device, 1, &in_flight_fences[get_current_frame()]);
		vkResetCommandBuffer(command_buffers[get_current_frame()], 0);

		return true;
	}

	void Swapchain::present()
	{
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = { image_available_semaphores[get_current_frame()] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[get_current_frame()];

		VkSemaphore signal_semaphores[] = { render_finished_semaphores[get_current_frame()] };
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		verify(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[get_current_frame()]));

		VkPresentInfoKHR present_info_khr {};
		present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		present_info_khr.waitSemaphoreCount = 1;
		present_info_khr.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = { swapchain };
		present_info_khr.swapchainCount = 1;
		present_info_khr.pSwapchains = swap_chains;
		present_info_khr.pImageIndices = &image_index;

		auto result = vkQueuePresentKHR(present_queue, &present_info_khr);

		auto was_resized = window->was_resized();
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || was_resized) {
			recreate_swapchain();
			window->reset_resize_status();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		current_frame = (current_frame + 1) % Config::max_frames_in_flight;
	}

	void Swapchain::recreate_swapchain(Ref<Disarray::Swapchain> old, bool should_clean)
	{
		window->wait_for_minimisation();
		swapchain_needs_recreation = true;

		wait_for_cleanup(device);

		if (should_clean)
			cleanup_swapchain();

		const auto& [capabilities, formats, present_modes] = resolve_swapchain_support(physical_device, window->get_surface());
		format = decide_surface_format(formats);
		present_mode = decide_present_mode(present_modes);
		extent = determine_extent(window, capabilities);

		std::uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = supply_cast<Vulkan::Surface>(window->get_surface());
		create_info.minImageCount = image_count;
		create_info.imageFormat = format.format;
		create_info.imageColorSpace = format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndex indices(physical_device, window->get_surface());
		uint32_t queue_family_indices[] = { indices.get_graphics_family(), indices.get_present_family() };

		if (indices.get_graphics_family() != indices.get_present_family()) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		} else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		graphics_family = indices.get_graphics_family();

		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = old ? supply_cast<Vulkan::Swapchain>(old) : nullptr;

		verify(vkCreateSwapchainKHR(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &swapchain));
		Log::debug("Swapchain created!");

		vkGetSwapchainImagesKHR(supply_cast<Vulkan::Device>(device), swapchain, &image_count, nullptr);
		swapchain_images.resize(image_count);
		vkGetSwapchainImagesKHR(supply_cast<Vulkan::Device>(device), swapchain, &image_count, swapchain_images.data());

		Log::debug("Swapchain images retrieved!");

		swapchain_image_views.resize(image_count);
		for (auto i = 0; i < swapchain_images.size(); i++) {
			VkImageViewCreateInfo image_view_create_info {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = swapchain_images[i];
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = format.format;
			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;

			verify(vkCreateImageView(supply_cast<Vulkan::Device>(device), &image_view_create_info, nullptr, &swapchain_image_views[i]));
		}

		create_synchronisation_objects();

		auto count = swapchain_images.size();
		if (count > Config::max_frames_in_flight)
			count = Config::max_frames_in_flight;

		VkCommandPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = graphics_family;

		verify(vkCreateCommandPool(supply_cast<Vulkan::Device>(device), &pool_info, nullptr, &command_pool));
		command_buffers.resize(count);
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = count;
		verify(vkAllocateCommandBuffers(supply_cast<Vulkan::Device>(device), &alloc_info, command_buffers.data()));
	}

	void Swapchain::cleanup_swapchain() {
		const auto vk_device = supply_cast<Vulkan::Device>(device);

		vkDestroyCommandPool(vk_device, command_pool, nullptr);

		for (size_t i = 0; i < Config::max_frames_in_flight; i++) {
			vkDestroySemaphore(vk_device, render_finished_semaphores[i], nullptr);
			vkDestroySemaphore(vk_device, image_available_semaphores[i], nullptr);
			vkDestroyFence(vk_device, in_flight_fences[i], nullptr);
		}

		for (size_t i = 0; i < swapchain_image_views.size(); i++) {
			vkDestroyImageView(vk_device, swapchain_image_views[i], nullptr);
		}

		vkDestroySwapchainKHR(vk_device, swapchain, nullptr);
	}

} // namespace Disarray::Vulkan