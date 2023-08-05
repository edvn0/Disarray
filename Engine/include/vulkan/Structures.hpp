#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Disarray::Vulkan {

	template <class T> struct vk_structures {
		T operator()() { throw std::runtime_error("Not implemented!"); };
	};

	template <> struct vk_structures<VkSubmitInfo> {
		VkSubmitInfo operator()()
		{
			VkSubmitInfo info {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			return info;
		}
	};

	template <> struct vk_structures<VkPresentInfoKHR> {
		VkPresentInfoKHR operator()()
		{
			VkPresentInfoKHR info {};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			return info;
		}
	};

} // namespace Disarray::Vulkan
