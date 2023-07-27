#pragma once

#include "graphics/PhysicalDevice.hpp"

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class ExtensionSupport {
	public:
		ExtensionSupport(VkPhysicalDevice device);

		operator bool() const { return valid; }

	private:
		bool valid { false };
	};

} // namespace Disarray::Vulkan