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

namespace Disarray::Vulkan {

	PhysicalDevice::PhysicalDevice(Disarray::Instance& inst, Disarray::Surface& surf)
	{
		static auto is_device_suitable = [](VkPhysicalDevice device, Disarray::Surface& surf) {
			Vulkan::QueueFamilyIndex indices(device, surf);
			ExtensionSupport extension_support(device);

			bool swapchain_is_allowed = false;
			if (extension_support) {
				const auto&& [capabilities, formats, modes] = resolve_swapchain_support(device, surf);
				swapchain_is_allowed = !formats.empty() && !modes.empty();
			}
			return indices && extension_support && swapchain_is_allowed;
		};

		auto& instance = cast_to<Vulkan::Instance>(inst);
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

		queue_family_index = make_ref<Vulkan::QueueFamilyIndex>(physical_device, surf);
	}

	PhysicalDevice::~PhysicalDevice() { }

} // namespace Disarray::Vulkan
