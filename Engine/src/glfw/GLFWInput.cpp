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

	bool Input::button_pressed(MouseCode code) { return glfwGetMouseButton(window_data->window, static_cast<int>(code)) == GLFW_PRESS; }

	bool Input::button_pressed(KeyCode code) { return glfwGetKey(window_data->window, static_cast<int>(code)) == GLFW_PRESS; }

} // namespace Disarray
