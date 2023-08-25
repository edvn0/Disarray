#include <GLFW/glfw3.h>

#include "core/Clock.hpp"

namespace Disarray {

float Clock::ms() { return glfwGetTime() * 1000.0f; }
float Clock::ns() { return glfwGetTime() * 1e6f; }

} // namespace Disarray
