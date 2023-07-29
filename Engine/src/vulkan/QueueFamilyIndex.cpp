#include "vulkan/QueueFamilyIndex.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Surface.hpp"

namespace Disarray::Vulkan {

	QueueFamilyIndex::QueueFamilyIndex(Ref<Disarray::PhysicalDevice> dev, Ref<Disarray::Surface> sur)
		: QueueFamilyIndex(cast_to<Vulkan::PhysicalDevice>(dev)->supply(), sur)
	{
	}

	QueueFamilyIndex::QueueFamilyIndex(VkPhysicalDevice device, Ref<Disarray::Surface> sur)
	{
		auto surface = cast_to<Vulkan::Surface>(sur);

		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);

		std::vector<VkQueueFamilyProperties> families(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families.data());

		std::uint32_t family_index = 0;
		for (const auto& family : families) {
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				get_graphics().emplace(family_index);
			}

			if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				get_compute().emplace(family_index);
			}

			if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
				get_transfer().emplace(family_index);
			}

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, family_index, surface->supply(), &present_support);
			if (present_support) {
				get_present().emplace(family_index);
			}

			if (is_complete())
				break;

			family_index++;
		}
	}

} // namespace Disarray::Vulkan
