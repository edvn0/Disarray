#include "DisarrayPCH.hpp"

#include "vulkan/ExtensionSupport.hpp"

#include <set>
#include <vector>

#include "vulkan/Config.hpp"
#include "vulkan/PhysicalDevice.hpp"

namespace Disarray::Vulkan {

ExtensionSupport::ExtensionSupport(VkPhysicalDevice device)
{
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available_extensions.data());

	std::set<std::string, std::less<>> required_extensions(Config::device_extensions.begin(), Config::device_extensions.end());

	for (const auto& extension : available_extensions) {
		required_extensions.erase(extension.extensionName);
	}

	valid = required_extensions.empty();
}

} // namespace Disarray::Vulkan
