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
	DISARRAY_LOG_DEBUG("Surface", "{}", "Surface created!");
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(*instance, surface, nullptr);
	DISARRAY_LOG_DEBUG("Surface", "{}", "Surface destroyed.");
}

} // namespace Disarray::Vulkan
