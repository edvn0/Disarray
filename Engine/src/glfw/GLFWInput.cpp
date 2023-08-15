#include "core/Input.hpp"

#include <GLFW/glfw3.h>

namespace Disarray {

struct WindowData {
	GLFWwindow* window;
};

static std::unique_ptr<WindowData> window_data { nullptr };

void Input::construct(UsageBadge<App>, Disarray::Window& window)
{
	window_data = std::make_unique<WindowData>();
	window_data->window = static_cast<GLFWwindow*>(window.native());
}

void Input::destruct() { window_data.reset(); }

glm::vec2 Input::mouse_position()
{
	double xpos, ypos;
	glfwGetCursorPos(window_data->window, &xpos, &ypos);
	return glm::vec2 { xpos, ypos };
}

bool Input::button_pressed(MouseCode code) { return glfwGetMouseButton(window_data->window, static_cast<int>(code)) == GLFW_PRESS; }

bool Input::key_pressed(KeyCode code) { return glfwGetKey(window_data->window, static_cast<int>(code)) == GLFW_PRESS; }

bool Input::button_released(MouseCode code) { return glfwGetMouseButton(window_data->window, static_cast<int>(code)) == GLFW_RELEASE; }

bool Input::key_released(KeyCode code) { return glfwGetKey(window_data->window, static_cast<int>(code)) == GLFW_RELEASE; }

} // namespace Disarray
