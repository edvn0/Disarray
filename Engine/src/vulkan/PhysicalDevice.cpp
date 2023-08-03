#include "DisarrayPCH.hpp"

#include "vulkan/PhysicalDevice.hpp"

#include "graphics/Instance.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/ExtensionSupport.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/QueueFamilyIndex.hpp"
#include "vulkan/SwapchainUtilities.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	PhysicalDevice::PhysicalDevice(Disarray::Instance& inst, Disarray::Surface& surf)
	{
		static auto is_device_suitable = [](VkPhysicalDevice device, Disarray::Surface& s) {
			Vulkan::QueueFamilyIndex indices(device, s);
			ExtensionSupport extension_support(device);

			bool swapchain_is_allowed = false;
			if (extension_support) {
				const auto&& [capabilities, formats, modes] = resolve_swapchain_support(device, s);
				swapchain_is_allowed = !formats.empty() && !modes.empty();
			} else {
				Log::error("PhysicalDevice", "Extension support is missing.");
			}
			return indices && extension_support && swapchain_is_allowed;
		};

		const auto& instance = cast_to<Vulkan::Instance>(inst);
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(*instance, &device_count, nullptr);

		if (device_count == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(*instance, &device_count, devices.data());

		for (const auto& device : devices) {
			if (is_device_suitable(device, surf)) {
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		vkGetPhysicalDeviceProperties(physical_device, &device_properties);

		queue_family_index = make_ref<Vulkan::QueueFamilyIndex>(physical_device, surf);
	}

	PhysicalDevice::~PhysicalDevice() = default;

} // namespace Disarray::Vulkan
