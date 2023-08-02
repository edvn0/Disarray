#include "vulkan/Swapchain.hpp"

#include "core/CleanupAwaiter.hpp"
#include "core/Log.hpp"
#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/SwapchainUtilities.hpp"
#include "vulkan/Verify.hpp"

#include <algorithm>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	Swapchain::Swapchain(Disarray::Window& win, Disarray::Device& dev, Disarray::Swapchain* old)
		: window(win)
		, device(dev)
	{
		recreate_swapchain(old, false);
		swapchain_needs_recreation = false;

		present_queue = cast_to<Vulkan::Device>(device).get_present_queue();
		graphics_queue = cast_to<Vulkan::Device>(device).get_graphics_queue();
	}

	Swapchain::~Swapchain() { cleanup_swapchain(); }

	void Swapchain::create_synchronisation_objects()
	{
		image_available_semaphores.resize(image_count());
		render_finished_semaphores.resize(image_count());
		in_flight_fences.resize(image_count());

		VkSemaphoreCreateInfo semaphore_info {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		const auto vk_device = supply_cast<Vulkan::Device>(device);

		for (std::uint32_t i = 0; i < image_count(); i++) {
			verify(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &image_available_semaphores[i]));
			verify(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &render_finished_semaphores[i]));
			verify(vkCreateFence(vk_device, &fence_create_info, nullptr, &in_flight_fences[i]));
		}
	}

	bool Swapchain::prepare_frame()
	{
		auto vk_device = supply_cast<Vulkan::Device>(device);

		vkWaitForFences(vk_device, 1, &in_flight_fences[get_current_frame()], VK_TRUE, UINT64_MAX);

		auto result
			= vkAcquireNextImageKHR(vk_device, swapchain, UINT64_MAX, image_available_semaphores[get_current_frame()], VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain();
			return false;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		verify(vkResetFences(vk_device, 1, &in_flight_fences[get_current_frame()]));
		verify(vkResetCommandPool(vk_device, command_buffers[get_current_frame()].command_pool, 0));

		return true;
	}

	void Swapchain::present()
	{
		VkSubmitInfo submit_info { VK_STRUCTURE_TYPE_SUBMIT_INFO };

		VkSemaphore wait_semaphores[] = { image_available_semaphores[get_current_frame()] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[get_current_frame()].buffer;

		VkSemaphore signal_semaphores[] = { render_finished_semaphores[get_current_frame()] };
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		verify(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[get_current_frame()]));

		VkPresentInfoKHR present_info_khr { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		present_info_khr.waitSemaphoreCount = 1;
		present_info_khr.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = { swapchain };
		present_info_khr.swapchainCount = 1;
		present_info_khr.pSwapchains = swap_chains;
		present_info_khr.pImageIndices = &image_index;

		auto result = vkQueuePresentKHR(present_queue, &present_info_khr);

		auto was_resized = window.was_resized();
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || was_resized) {
			recreate_swapchain();
			window.reset_resize_status();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		current_frame = (current_frame + 1) % image_count();
		verify(vkWaitForFences(supply_cast<Vulkan::Device>(device), 1, &in_flight_fences[current_frame], true, UINT64_MAX));
	}

	void Swapchain::recreate_swapchain(Disarray::Swapchain* old, bool should_clean)
	{
		window.wait_for_minimisation();
		swapchain_needs_recreation = true;

		wait_for_cleanup(device);

		if (should_clean) {
			cleanup_swapchain();
		}

		render_pass = make_ref<Vulkan::RenderPass>(device,
			RenderPassProperties {
				.image_format = ImageFormat::SBGR,
				.keep_depth = false,
				.has_depth = false,
				.should_present = true,
				.debug_name { "Swapchain RenderPass" },
			});

		const auto& [capabilities, formats, present_modes]
			= resolve_swapchain_support(supply_cast<Vulkan::PhysicalDevice>(device.get_physical_device()), window.get_surface());
		format = decide_surface_format(formats);
		present_mode = decide_present_mode(present_modes);
		extent = determine_extent(window, capabilities);

		std::uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = supply_cast<Vulkan::Surface>(window.get_surface());
		create_info.minImageCount = image_count;
		create_info.imageFormat = format.format;
		create_info.imageColorSpace = format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndex indices(device.get_physical_device(), window.get_surface());
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
		auto* old_vulkan = static_cast<Vulkan::Swapchain*>(old);
		create_info.oldSwapchain = old ? old_vulkan->supply() : nullptr;

		verify(vkCreateSwapchainKHR(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &swapchain));

		vkGetSwapchainImagesKHR(supply_cast<Vulkan::Device>(device), swapchain, &image_count, nullptr);
		swapchain_images.resize(image_count);
		vkGetSwapchainImagesKHR(supply_cast<Vulkan::Device>(device), swapchain, &image_count, swapchain_images.data());

		swapchain_image_views.resize(image_count);
		for (std::size_t i = 0; i < swapchain_images.size(); i++) {
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

		VkCommandPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		pool_info.queueFamilyIndex = graphics_family;

		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		command_buffers.resize(image_count);
		for (auto& cmd_buffer : command_buffers) {
			verify(vkCreateCommandPool(supply_cast<Vulkan::Device>(device), &pool_info, nullptr, &cmd_buffer.command_pool));
			alloc_info.commandPool = cmd_buffer.command_pool;
			verify(vkAllocateCommandBuffers(supply_cast<Vulkan::Device>(device), &alloc_info, &cmd_buffer.buffer));
		}

		recreate_framebuffer(should_clean);
	}

	void Swapchain::cleanup_swapchain()
	{
		const auto vk_device = supply_cast<Vulkan::Device>(device);

		render_pass.reset();

		for (auto& fb : framebuffers) {
			vkDestroyFramebuffer(vk_device, fb, nullptr);
		}

		for (auto& [_, command_pool] : command_buffers)
			vkDestroyCommandPool(vk_device, command_pool, nullptr);

		for (size_t i = 0; i < image_count(); i++) {
			vkDestroySemaphore(vk_device, render_finished_semaphores[i], nullptr);
			vkDestroySemaphore(vk_device, image_available_semaphores[i], nullptr);
			vkDestroyFence(vk_device, in_flight_fences[i], nullptr);
		}

		for (size_t i = 0; i < swapchain_image_views.size(); i++) {
			vkDestroyImageView(vk_device, swapchain_image_views[i], nullptr);
		}

		vkDestroySwapchainKHR(vk_device, swapchain, nullptr);
	}

	Disarray::RenderPass& Swapchain::get_render_pass() { return *render_pass; }

	void Swapchain::recreate_framebuffer(bool should_clean)
	{
		const auto vk_device = supply_cast<Vulkan::Device>(device);

		if (should_clean) {
			for (auto& fb : framebuffers) {
				vkDestroyFramebuffer(vk_device, fb, nullptr);
			}
		}

		framebuffers.resize(image_count());

		std::uint32_t i { 0 };
		for (auto& fb : framebuffers) {
			VkImageView attachments[] = { swapchain_image_views[i++] };

			VkFramebufferCreateInfo fb_create_info {};
			fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fb_create_info.renderPass = render_pass->supply();
			fb_create_info.attachmentCount = 1;
			fb_create_info.pAttachments = attachments;
			fb_create_info.width = extent.width;
			fb_create_info.height = extent.height;
			fb_create_info.layers = 1;

			verify(vkCreateFramebuffer(vk_device, &fb_create_info, nullptr, &fb));
		}
	}

} // namespace Disarray::Vulkan
