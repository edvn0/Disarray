#include "core/Clock.hpp"

#include <glfw/glfw3.h>

namespace Disarray {

	float Clock::ms() { return glfwGetTime() * 1000.0f; }
	float Clock::ns() { return glfwGetTime() * 1e6f; }

} // namespace Disarray
