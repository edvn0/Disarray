#include "vulkan/Window.hpp"

#include "core/Log.hpp"
#include "vulkan/Swapchain.hpp"

#include <glfw/glfw3.h>
#include <iostream>
#include <string>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	Window::Window(std::uint32_t w, std::uint32_t h)
		: Disarray::Window(w, h)
	{
		if (const auto initialised = glfwInit(); !initialised) {
			throw;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(get_width(), get_height(), "Vulkan engine", nullptr, nullptr);

		if (window == nullptr)
			throw;

		instance = make_ref<Vulkan::Instance>();
		surface = make_ref<Vulkan::Surface>(instance, window);

		glfwSetKeyCallback(window, [](GLFWwindow* handle, int key, int scancode, int action, int mods) {
			if (key == GLFW_KEY_ESCAPE)
				glfwSetWindowShouldClose(handle, true);
		});
	}

	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		Log::debug("Window destroyed. GLFW terminated.");
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

} // namespace Disarray::Vulkan
