#include <GLFW/glfw3.h>

#include "core/Clock.hpp"

namespace Disarray {

auto Clock::ms() -> double { return glfwGetTime() * 1000.0F; }
auto Clock::ns() -> double { return glfwGetTime() * 1E6F; }

} // namespace Disarray
