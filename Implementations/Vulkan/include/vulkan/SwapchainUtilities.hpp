#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <magic_enum.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Surface.hpp"

namespace Disarray::Vulkan {

struct ResolvedSwapchainSupport {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> surface_formats;
	std::vector<VkPresentModeKHR> present_modes;
	VkSampleCountFlagBits msaa { VK_SAMPLE_COUNT_1_BIT };
};

inline auto resolve_swapchain_support(VkPhysicalDevice device, Disarray::Surface& surf) -> ResolvedSwapchainSupport;

inline auto resolve_swapchain_support(Disarray::PhysicalDevice& device, Disarray::Surface& surf) -> ResolvedSwapchainSupport
{
	return resolve_swapchain_support(supply_cast<Vulkan::PhysicalDevice>(device), surf);
}

inline auto resolve_swapchain_support(VkPhysicalDevice physical_device, Disarray::Surface& surf) -> ResolvedSwapchainSupport
{
	auto* surface = supply_cast<Vulkan::Surface>(surf);
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

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_2_BIT) {
		support.msaa = VK_SAMPLE_COUNT_2_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT) {
		support.msaa = VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT) {
		support.msaa = VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT) {
		support.msaa = VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT) {
		support.msaa = VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_64_BIT) {
		support.msaa = VK_SAMPLE_COUNT_64_BIT;
	}

	return support;
}

inline auto decide_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) -> VkSurfaceFormatKHR
{
	for (const auto& format : formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	// TODO(edvn0): Optimise this decision later on!
	return formats[0];
}

inline auto decide_present_mode(const std::vector<VkPresentModeKHR>& present_modes) -> VkPresentModeKHR
{
	for (const auto& present_mode : present_modes) {
		if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return present_mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

inline auto determine_extent(Disarray::Window& window, const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	const auto&& [width, height] = window.get_framebuffer_size();

	VkExtent2D actual_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

	actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actual_extent;
}

} // namespace Disarray::Vulkan
