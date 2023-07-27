#include "vulkan/Swapchain.hpp"

#include "core/Log.hpp"
#include "core/Types.hpp"
#include "core/Window.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/SwapchainUtilities.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	Swapchain::Swapchain(Scope<Disarray::Window>& window, Ref<Disarray::Device> dev, Ref<Disarray::PhysicalDevice> physical_device)
		: device(dev)
	{
		const auto& [capabilities, formats, present_modes] = resolve_swapchain_support(physical_device, window->get_surface());
		auto format = decide_surface_format(formats);
		auto present_mode = decide_present_mode(present_modes);
		auto extent = determine_extent(window, capabilities);

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

		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		verify(vkCreateSwapchainKHR(supply_cast<Vulkan::Device>(device), &create_info, nullptr, &swapchain));
		Log::debug("Swapchain created!");
	}

	Swapchain::~Swapchain() {
		vkDestroySwapchainKHR(supply_cast<Vulkan::Device>(device), swapchain, nullptr);
		Log::debug("Swapchain destroyed!");
	}

}