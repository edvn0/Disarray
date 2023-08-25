#pragma once

#include <vulkan/vulkan.h>

#include "graphics/PhysicalDevice.hpp"

namespace Disarray::Vulkan {

class ExtensionSupport {
public:
	ExtensionSupport(VkPhysicalDevice device);

	operator bool() const { return valid; }

private:
	bool valid { false };
};

} // namespace Disarray::Vulkan
