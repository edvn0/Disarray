#include "DisarrayPCH.hpp"

#include "vulkan/Device.hpp"

#include "core/Window.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Verify.hpp"

#include <set>

namespace Disarray::Vulkan {

	Device::Device(Disarray::Window& window)
		: physical_device(PhysicalDevice::construct(window.get_instance(), window.get_surface()))
	{
		auto& queue_family_index = physical_device->get_queue_family_indexes();
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_families { queue_family_index.get_graphics_family(), queue_family_index.get_present_family() };

		float prio = 1.0f;
		for (uint32_t family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &prio;
			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceFeatures features {};
		features.wideLines = true;
		features.logicOp = true;
		features.pipelineStatisticsQuery = true;

		VkDeviceCreateInfo device_create_info {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = static_cast<std::uint32_t>(queue_create_infos.size());
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		device_create_info.pEnabledFeatures = &features;
		device_create_info.enabledExtensionCount = static_cast<std::uint32_t>(Config::device_extensions.size());
		device_create_info.ppEnabledExtensionNames = Config::device_extensions.data();

		const auto& vk_device = cast_to<Vulkan::PhysicalDevice>(*physical_device);
		verify(vkCreateDevice(vk_device.supply(), &device_create_info, nullptr, &device));

		vkGetDeviceQueue(device, queue_family_index.get_graphics_family(), 0, &graphics);
		vkGetDeviceQueue(device, queue_family_index.get_present_family(), 0, &present);

		// The debug marker extension is not part of the core, so function pointers need to be loaded manually.
		auto vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
		auto vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
		auto vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
		auto vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
		auto vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
	}

	Device::~Device()
	{
		vkDestroyDevice(device, nullptr);
		Log::debug("Device", "Device destroyed.");
	}

} // namespace Disarray::Vulkan
