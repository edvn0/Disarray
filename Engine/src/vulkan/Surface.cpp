#include "vulkan/Surface.hpp"

#include "core/Log.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Verify.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	Surface::Surface(Ref<Instance> inst, GLFWwindow* window)
		: instance(inst)
	{
		verify(glfwCreateWindowSurface(instance->get(), window, nullptr, &surface));
		Log::debug("Surface created!");
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(instance->get(), surface, nullptr);
		Log::debug("Surface destroyed.");
	}

} // namespace Disarray::Vulkan
