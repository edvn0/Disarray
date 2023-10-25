#include "DisarrayPCH.hpp"

#include "vulkan/Device.hpp"

#include <set>

#include "core/Window.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "vulkan/Config.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Verify.hpp"

namespace Disarray::Vulkan {

Device::Device(Disarray::Window& window)
	: physical_device(PhysicalDevice::construct(window.get_instance(), window.get_surface()))
{
	auto& queue_family_index = physical_device->get_queue_family_indexes();
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<uint32_t> unique_queue_families { queue_family_index.get_graphics_family(), queue_family_index.get_present_family() };

	float prio = 1.0F;
	for (uint32_t family : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = family;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &prio;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures features {};
#ifdef DISARRAY_WINDOWS
	features.wideLines = VK_TRUE;
	features.logicOp = VK_TRUE;
	features.pipelineStatisticsQuery = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;
	features.independentBlend = VK_TRUE;
#endif

	VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
	shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
	shader_draw_parameters_features.pNext = nullptr;
	shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

	VkDeviceCreateInfo device_create_info {};
	device_create_info.pNext = &shader_draw_parameters_features;
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<std::uint32_t>(queue_create_infos.size());
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.pEnabledFeatures = &features;
	std::array extensions = { Config::device_extensions[0].data(), Config::device_extensions[1].data() };
	device_create_info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
	device_create_info.ppEnabledExtensionNames = extensions.data();

	const auto& vk_device = cast_to<Vulkan::PhysicalDevice>(*physical_device);
	verify(vkCreateDevice(vk_device.supply(), &device_create_info, nullptr, &device));

	vkGetDeviceQueue(device, queue_family_index.get_graphics_family(), 0, &graphics);
	vkGetDeviceQueue(device, queue_family_index.get_present_family(), 0, &present);
}

Device::~Device() { vkDestroyDevice(device, nullptr); }

} // namespace Disarray::Vulkan
