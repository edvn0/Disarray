#include "vulkan/Device.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/Verify.hpp"
#include <set>

namespace Disarray::Vulkan {

	Device::Device(Ref<Disarray::PhysicalDevice> physical, Ref<Disarray::Surface> surface)
	{
		QueueFamilyIndex indices(physical, surface);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_families  {indices.get_graphics_family(), indices.get_present_family()};

		float prio = 1.0f;
		for (uint32_t family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &prio;
			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures features {};
		features.wideLines = true;

		VkDeviceCreateInfo device_create_info {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = queue_create_infos.size();
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		device_create_info.pEnabledFeatures = &features;
		device_create_info.enabledExtensionCount = Config::device_extensions.size();
		device_create_info.ppEnabledExtensionNames = Config::device_extensions.data();

		const auto physical_device = cast_to<Vulkan::PhysicalDevice>(physical);
		verify(vkCreateDevice(physical_device->get(), &device_create_info, nullptr, &device));

		vkGetDeviceQueue(device, indices.get_graphics_family(), 0, &graphics);
		vkGetDeviceQueue(device, indices.get_present_family(), 0, &present);

	}

	Device::~Device() { vkDestroyDevice(device, nullptr); }

} // namespace Disarray::Vulkan
