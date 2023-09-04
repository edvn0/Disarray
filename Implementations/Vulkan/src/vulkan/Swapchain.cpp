#include "DisarrayPCH.hpp"

#include "graphics/Swapchain.hpp"

#include <vulkan/vulkan.h>

#include <magic_enum.hpp>

#include <algorithm>

#include "core/CleanupAwaiter.hpp"
#include "core/Log.hpp"
#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/RenderPass.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/DebugMarker.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Framebuffer.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/SwapchainUtilities.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

namespace Disarray::Vulkan {

Swapchain::Swapchain(Disarray::Window& win, Disarray::Device& dev, Disarray::Swapchain* old)
	: window(win)
	, device(dev)
	, present_queue(cast_to<Vulkan::Device>(device).get_present_queue())
	, graphics_queue(cast_to<Vulkan::Device>(device).get_graphics_queue())
{
#ifdef HAS_MSAA
	samples = cast_to<Vulkan::PhysicalDevice>(dev.get_physical_device()).get_sample_count();
#else
#endif
	recreate_swapchain(old, false);
	DISARRAY_LOG_DEBUG(
		"Swapchain", "Swapchain created with extent and format: {}, {}; {}", extent.width, extent.height, magic_enum::enum_name(format.format));
	swapchain_needs_recreation = false;
}

Swapchain::~Swapchain()
{
	cleanup_swapchain();
	DISARRAY_LOG_DEBUG("Swapchain", "{}", "Swapchain destroyed.");
}

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

	auto* vk_device = supply_cast<Vulkan::Device>(device);

	for (std::uint32_t i = 0; i < image_count(); i++) {
		verify(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &image_available_semaphores[i]));
		verify(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &render_finished_semaphores[i]));
		verify(vkCreateFence(vk_device, &fence_create_info, nullptr, &in_flight_fences[i]));
	}
}

auto Swapchain::prepare_frame() -> bool
{
	auto* vk_device = supply_cast<Vulkan::Device>(device);

	vkWaitForFences(vk_device, 1, &in_flight_fences[get_current_frame()], VK_TRUE, UINT64_MAX);

	auto result
		= vkAcquireNextImageKHR(vk_device, swapchain, UINT64_MAX, image_available_semaphores[get_current_frame()], VK_NULL_HANDLE, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain();
		return false;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw SwapchainImageAcquisitionException("failed to acquire swap chain image!");
	}

	verify(vkResetFences(vk_device, 1, &in_flight_fences[get_current_frame()]));
	verify(vkResetCommandPool(vk_device, command_buffers[get_current_frame()].command_pool, 0));

	return true;
}

void Swapchain::present()
{
	auto submit_info = vk_structures<VkSubmitInfo> {}();

	std::array<VkSemaphore, 1> wait_semaphores = { image_available_semaphores[get_current_frame()] };
	std::array<VkPipelineStageFlags, 1> wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.pWaitSemaphores = wait_semaphores.data();
	submit_info.waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores.size());
	submit_info.pWaitDstStageMask = wait_stages.data();
	submit_info.commandBufferCount = static_cast<std::uint32_t>(wait_stages.size());
	submit_info.pCommandBuffers = &command_buffers[get_current_frame()].buffer;

	std::array<VkSemaphore, 1> signal_semaphores = { render_finished_semaphores[get_current_frame()] };
	submit_info.signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size());
	submit_info.pSignalSemaphores = signal_semaphores.data();

	verify(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[get_current_frame()]));

	auto present_info_khr = vk_structures<VkPresentInfoKHR> {}();
	present_info_khr.waitSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size());
	present_info_khr.pWaitSemaphores = signal_semaphores.data();

	std::array<VkSwapchainKHR, 1> swap_chains = { swapchain };
	present_info_khr.swapchainCount = static_cast<std::uint32_t>(swap_chains.size());
	present_info_khr.pSwapchains = swap_chains.data();
	present_info_khr.pImageIndices = &image_index;

	auto result = vkQueuePresentKHR(present_queue, &present_info_khr);

	if (auto was_resized = window.was_resized(); was_resized || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreate_swapchain();
		window.reset_resize_status();
	} else if (result != VK_SUCCESS) {
		throw CouldNotPresentSwapchainException("failed to present swap chain image!");
	}

	current_frame = (current_frame + 1) % image_count();
	verify(vkWaitForFences(supply_cast<Vulkan::Device>(device), 1, &in_flight_fences[current_frame], true, UINT64_MAX));
}

void Swapchain::recreate_renderpass()
{
	// Render Pass
	VkAttachmentDescription colour_attachment_description = {};
	// Color attachment
	colour_attachment_description.format = VK_FORMAT_B8G8R8A8_SRGB;
	colour_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	colour_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colour_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colour_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colour_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colour_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colour_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colour_reference = {};
	colour_reference.attachment = 0;
	colour_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &colour_reference;
	subpass_description.inputAttachmentCount = 0;
	subpass_description.pInputAttachments = nullptr;
	subpass_description.preserveAttachmentCount = 0;
	subpass_description.pPreserveAttachments = nullptr;
	subpass_description.pResolveAttachments = nullptr;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &colour_attachment_description;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	render_pass = RenderPass::construct(device);
	auto& vk_render_pass = cast_to<Vulkan::RenderPass>(*render_pass);
	vk_render_pass.create_with(render_pass_info);
}

void Swapchain::recreate_swapchain(Disarray::Swapchain* old, bool should_clean)
{
	window.wait_for_minimisation();
	swapchain_needs_recreation = true;

	wait_for_cleanup(device);

	if (should_clean) {
		cleanup_swapchain();
	}

	recreate_renderpass();

	const auto& [capabilities, formats, present_modes, msaa]
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
	std::array<uint32_t, 2> queue_family_indices = { indices.get_graphics_family(), indices.get_present_family() };

	if (indices.get_graphics_family() != indices.get_present_family()) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices.size());
		create_info.pQueueFamilyIndices = queue_family_indices.data();
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	graphics_family = indices.get_graphics_family();

	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	const auto* old_vulkan = dynamic_cast<const Vulkan::Swapchain*>(old);
	create_info.oldSwapchain = old != nullptr ? old_vulkan->supply() : nullptr;

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

	recreate_framebuffer();
}

void Swapchain::cleanup_swapchain()
{
	auto* vk_device = supply_cast<Vulkan::Device>(device);

	render_pass.reset();

	for (auto& fb : framebuffers) {
		vkDestroyFramebuffer(vk_device, fb, nullptr);
	}

	for (auto& [_, command_pool] : command_buffers) {
		vkDestroyCommandPool(vk_device, command_pool, nullptr);
	}

	for (size_t i = 0; i < image_count(); i++) {
		vkDestroySemaphore(vk_device, render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(vk_device, image_available_semaphores[i], nullptr);
		vkDestroyFence(vk_device, in_flight_fences[i], nullptr);
	}

	for (auto& swapchain_image_view : swapchain_image_views) {
		vkDestroyImageView(vk_device, swapchain_image_view, nullptr);
	}

	vkDestroySwapchainKHR(vk_device, swapchain, nullptr);
}

auto Swapchain::get_render_pass() -> Disarray::RenderPass& { return *render_pass; }

void Swapchain::recreate_framebuffer()
{
	auto* const vk_device = supply_cast<Vulkan::Device>(device);

	framebuffers.resize(image_count());

	std::uint32_t i { 0 };
	for (auto& fb : framebuffers) {
		std::array<VkImageView, 1> attachments = { swapchain_image_views[i++] };

		VkFramebufferCreateInfo fb_create_info {};
		fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create_info.renderPass = supply_cast<Vulkan::RenderPass>(*render_pass);
		fb_create_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
		fb_create_info.pAttachments = attachments.data();
		fb_create_info.width = extent.width;
		fb_create_info.height = extent.height;
		fb_create_info.layers = 1;

		verify(vkCreateFramebuffer(vk_device, &fb_create_info, nullptr, &fb));
	}
}
} // namespace Disarray::Vulkan
