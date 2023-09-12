#include <GLFW/glfw3.h>

#include "core/Clock.hpp"

namespace Disarray {

auto Clock::ms() -> float { return static_cast<float>(glfwGetTime()) * 1000.0F; }
auto Clock::ns() -> float { return static_cast<float>(glfwGetTime()) * 1E6F; }

} // namespace Disarray
