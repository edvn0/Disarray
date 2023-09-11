#include <GLFW/glfw3.h>

#include "core/Input.hpp"

namespace Disarray {

struct Input::WindowData {
	GLFWwindow* window;
};

auto Input::WindowDataDeleter::operator()(WindowData* ptr) -> void { delete ptr; }

void Input::construct(const Disarray::Window& window)
{
	window_data = make_scope<WindowData, WindowDataDeleter>();
	window_data->window = static_cast<GLFWwindow*>(window.native());
}

void Input::destruct() { window_data.reset(); }

auto Input::mouse_position() -> glm::vec2
{
	double xpos { 0 };
	double ypos { 0 };
	glfwGetCursorPos(window_data->window, &xpos, &ypos);
	return glm::vec2 { xpos, ypos };
}

auto Input::button_pressed(MouseCode code) -> bool { return glfwGetMouseButton(window_data->window, static_cast<int>(code)) == GLFW_PRESS; }

auto Input::key_pressed(KeyCode code) -> bool { return glfwGetKey(window_data->window, static_cast<int>(code)) == GLFW_PRESS; }

auto Input::button_released(MouseCode code) -> bool { return glfwGetMouseButton(window_data->window, static_cast<int>(code)) == GLFW_RELEASE; }

auto Input::key_released(KeyCode code) -> bool { return glfwGetKey(window_data->window, static_cast<int>(code)) == GLFW_RELEASE; }

} // namespace Disarray
