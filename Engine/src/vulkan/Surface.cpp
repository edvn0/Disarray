#include "DisarrayPCH.hpp"

#include "vulkan/Surface.hpp"

#include "core/Log.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Verify.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Disarray::Vulkan {

	Surface::Surface(Instance& inst, GLFWwindow* window)
		: instance(inst)
	{
		verify(glfwCreateWindowSurface(*instance, window, nullptr, &surface));
		Log::debug("Surface", "Surface created!");
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(*instance, surface, nullptr);
		Log::debug("Surface", "Surface destroyed.");
	}

} // namespace Disarray::Vulkan
