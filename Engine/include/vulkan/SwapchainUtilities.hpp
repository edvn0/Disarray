#pragma once

#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Surface.hpp"

#include <algorithm>
#include <cstdint>
#include <glfw/glfw3.h>
#include <limits>
#include <vector>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	struct ResolvedSwapchainSupport {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> surface_formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	static ResolvedSwapchainSupport resolve_swapchain_support(VkPhysicalDevice device, Disarray::Surface& surf);

	static ResolvedSwapchainSupport resolve_swapchain_support(Disarray::PhysicalDevice& device, Disarray::Surface& surf)
	{
		return resolve_swapchain_support(supply_cast<Vulkan::PhysicalDevice>(device), surf);
	}

	static ResolvedSwapchainSupport resolve_swapchain_support(VkPhysicalDevice physical_device, Disarray::Surface& surf)
	{
		auto surface = supply_cast<Vulkan::Surface>(surf);
		ResolvedSwapchainSupport support;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

		if (format_count != 0) {
			support.surface_formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, support.surface_formats.data());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

		if (present_mode_count != 0) {
			support.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, support.present_modes.data());
		}

		return support;
	}

	static VkSurfaceFormatKHR decide_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		for (const auto& format : formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}

		// TODO(edvn0): Optimise this decision later on!
		return formats[0];
	}

	static VkPresentModeKHR decide_present_mode(const std::vector<VkPresentModeKHR>& present_modes)
	{
		for (const auto& present_mode : present_modes) {
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return present_mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	static VkExtent2D determine_extent(Disarray::Window& window, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		} else {
			const auto&& [width, height] = window.get_framebuffer_size();

			VkExtent2D actual_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actual_extent;
		}
	}

} // namespace Disarray::Vulkan
