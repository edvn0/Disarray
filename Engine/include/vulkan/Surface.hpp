#pragma once

#include <vulkan/vulkan.h>

extern "C" {
struct GLFWwindow;
}

namespace Disarray::Vulkan {
	class Surface {
	public:
		Surface(GLFWwindow*);
		~Surface();

	private:
		VkSurfaceKHR surface;
	};
} // namespace Disarray::Vulkan
