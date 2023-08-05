#include "DisarrayPCH.hpp"

#include "vulkan/Window.hpp"

#include "GLFW/glfw3.h"
#include "core/App.hpp"
#include "core/Log.hpp"
#include "vulkan/Swapchain.hpp"

#include <string>

namespace Disarray::Vulkan {

	Window::Window(const Disarray::WindowProperties& properties)
		: Disarray::Window(properties)
	{
		if (const auto initialised = glfwInit(); !initialised) {
			throw;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		const auto& [width, height, name] = get_properties();
		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

		if (window == nullptr)
			throw;

		instance = make_scope<Vulkan::Instance>();
		surface = make_scope<Vulkan::Surface>(*instance, window);

		user_data = new UserData();

		glfwSetWindowUserPointer(window, &user_data);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* handle, int w, int h) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(handle));
			data.was_resized = true;
		});

		glfwSetKeyCallback(window, [](GLFWwindow* handle, int key, int scancode, int action, int mods) {
			if (key == GLFW_KEY_ESCAPE)
				glfwSetWindowShouldClose(handle, true);
		});
	}

	Window::~Window()
	{
		surface.reset();
		glfwDestroyWindow(window);
		glfwTerminate();
		Log::debug("Window", "Window destroyed.");
	}

	void Window::update() { glfwPollEvents(); }

	bool Window::should_close() const { return glfwWindowShouldClose(window); }

	std::pair<int, int> Window::get_framebuffer_size()
	{
		int width;
		int height;
		glfwGetFramebufferSize(window, &width, &height);
		return { width, height };
	}

	void Window::reset_resize_status() { user_data->was_resized = false; }

	bool Window::was_resized() const { return user_data->was_resized; }

	void Window::wait_for_minimisation()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
	}

	std::pair<float, float> Window::get_framebuffer_scale()
	{
		float width;
		float height;
		glfwGetWindowContentScale(window, &width, &height);
		return { width, height };
	}

} // namespace Disarray::Vulkan
