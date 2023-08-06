#include "DisarrayPCH.hpp"

#include "vulkan/Window.hpp"

#include "GLFW/glfw3.h"
#include "core/App.hpp"
#include "core/Log.hpp"
#include "vulkan/Swapchain.hpp"

#include <graphics/ImageLoader.hpp>
#include <string>

namespace Disarray::Vulkan {

	inline void key_callback(GLFWwindow* handle, int key, int scancode, int action, int mods)
	{
		if (action != GLFW_RELEASE)
			return;

		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(handle, true);
			return;
		}

		auto& user_data = *static_cast<UserData*>(glfwGetWindowUserPointer(handle));
		if (key == GLFW_KEY_F11) {
			// if we are in fullscreen already, don't!
			if (!user_data.fullscreen) {
				glfwGetWindowPos(handle, &user_data.pos_x, &user_data.pos_y);
				glfwSetWindowMonitor(handle, user_data.monitor, 0, 0, user_data.mode->width, user_data.mode->height, user_data.mode->refreshRate);
			} else {
				glfwSetWindowMonitor(handle, nullptr, user_data.pos_x, user_data.pos_y, user_data.width, user_data.height, 0);
			}
			user_data.fullscreen = !user_data.fullscreen;
			return;
		}
	}

	Window::Window(const Disarray::WindowProperties& properties)
		: Disarray::Window(properties)
	{
		if (const auto initialised = glfwInit(); !initialised) {
			throw;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		const auto& [width, height, name, is_fullscreen] = get_properties();

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		user_data.monitor = glfwGetPrimaryMonitor();
		user_data.width = width;
		user_data.height = height;
		user_data.mode = mode;

		if (is_fullscreen) {
			window = glfwCreateWindow(mode->width, mode->height, name.c_str(), glfwGetPrimaryMonitor(), nullptr);
			user_data.fullscreen = true;
		} else {
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
			user_data.fullscreen = false;

			glfwSetWindowPos(window, 100, 100);

			int px;
			int py;
			glfwGetWindowPos(window, &px, &py);
			user_data.pos_x = px;
			user_data.pos_y = py;
			glfwShowWindow(window);
		}

		{
			DataBuffer buffer;
			ImageLoader loader { "Assets/Icons/Disarray_Logo.png", buffer };
			std::array<GLFWimage, 1> images {};
			images[0].width = loader.get_extent().width;
			images[0].height = loader.get_extent().height;
			images[0].pixels = Disarray::bit_cast<unsigned char*>(buffer.get_data());
			glfwSetWindowIcon(window, 1, images.data());
		}

		if (window == nullptr)
			throw;

		instance = make_scope<Vulkan::Instance>();
		surface = make_scope<Vulkan::Surface>(*instance, window);

		glfwSetWindowUserPointer(window, &user_data);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* handle, int w, int h) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(handle));
			data.was_resized = true;
		});

		glfwSetKeyCallback(window, &key_callback);
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

	void Window::reset_resize_status() { user_data.was_resized = false; }

	bool Window::was_resized() const { return user_data.was_resized; }

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
