#include "DisarrayPCH.hpp"

#include <GLFW/glfw3.h>

#include <graphics/ImageLoader.hpp>

#include <string>

#include "core/App.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "core/exceptions/GeneralExceptions.hpp"
#include "core/filesystem/AssetLocations.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Window.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

namespace Disarray::Vulkan {

void Window::register_event_handler(Disarray::App& app)
{
	user_data.callback = [&app = app](Event& event) { app.on_event(event); };

	Input::construct(*this);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int, int) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));
		data.was_resized = true;
	});

	glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int new_width, int new_height) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));

		WindowResizeEvent event(static_cast<std::uint32_t>(new_width), static_cast<std::uint32_t>(new_height));
		data.callback(event);
		data.width = new_width;
		data.height = new_height;
	});

	glfwSetWindowCloseCallback(window, [](GLFWwindow* win) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));

		WindowCloseEvent event;
		data.callback(event);
	});

	glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int, int action, int) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));

		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(win, true);
			return;
		}

		if (key == GLFW_KEY_F11 && action == GLFW_RELEASE) {
			// if we are in fullscreen already, don't!
			if (!data.fullscreen) {
				glfwGetWindowPos(win, &data.pos_x, &data.pos_y);
				glfwSetWindowMonitor(win, data.monitor, 0, 0, data.mode->width, data.mode->height, data.mode->refreshRate);
			} else {
				glfwSetWindowMonitor(win, nullptr, data.pos_x, data.pos_y, data.width, data.height, 0);
			}
			data.fullscreen = !data.fullscreen;
			return;
		}

		switch (action) {
		case GLFW_PRESS: {
			KeyPressedEvent event(static_cast<KeyCode>(key), 0);
			data.callback(event);
			break;
		}
		case GLFW_RELEASE: {
			KeyReleasedEvent event(static_cast<KeyCode>(key));
			data.callback(event);
			break;
		}
		case GLFW_REPEAT: {
			KeyPressedEvent event(static_cast<KeyCode>(key), 1);
			data.callback(event);
			break;
		}
		}
	});

	glfwSetCharCallback(window, [](GLFWwindow* win, std::uint32_t codepoint) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));

		KeyTypedEvent event(static_cast<KeyCode>(codepoint));
		data.callback(event);
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));

		switch (action) {
		case GLFW_PRESS: {
			MouseButtonPressedEvent event(static_cast<MouseCode>(button));
			data.callback(event);
			break;
		}
		case GLFW_RELEASE: {
			MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
			data.callback(event);
			break;
		}
		}
	});

	glfwSetScrollCallback(window, [](GLFWwindow* win, double x_offset, double y_offset) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));

		MouseScrolledEvent event(static_cast<float>(x_offset), static_cast<float>(y_offset));
		data.callback(event);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* win, double cursor_x, double cursor_y) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));
		MouseMovedEvent event(static_cast<float>(cursor_x), static_cast<float>(cursor_y));
		data.callback(event);
	});

	glfwSetWindowIconifyCallback(window, [](GLFWwindow* win, int iconified) {
		auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(win));
		WindowMinimizeEvent event(static_cast<bool>(iconified));
		data.callback(event);
	});
}

Window::Window(const Disarray::WindowProperties& properties)
	: Disarray::Window(properties)
{
	if (const auto initialised = glfwInit(); initialised == GLFW_FALSE) {
		throw CouldNotInitialiseWindowingAPI("Could not initialise GLFW.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	const auto& [width, height, name, is_fullscreen, use_validation_layers] = get_properties();

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
		window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name.c_str(), nullptr, nullptr);
		user_data.fullscreen = false;

		glfwSetWindowPos(window, 100, 100);

		int pos_x {};
		int pos_y {};
		glfwGetWindowPos(window, &pos_x, &pos_y);
		user_data.pos_x = pos_x;
		user_data.pos_y = pos_y;
		glfwShowWindow(window);
	}

	{
		DataBuffer buffer;
		ImageLoader loader { FS::icon("Disarray_Logo.png"), buffer };
		std::array<GLFWimage, 1> images {};
		images[0].width = static_cast<int>(loader.get_extent().width);
		images[0].height = static_cast<int>(loader.get_extent().height);
		images[0].pixels = Disarray::bit_cast<unsigned char*>(buffer.get_data());
		glfwSetWindowIcon(window, 1, images.data());
	}

	if (window == nullptr) {
		throw WindowingAPIException("Window was nullptr");
	}

	instance = make_scope<Vulkan::Instance>(use_validation_layers);
	surface = make_scope<Vulkan::Surface>(*instance, window);

	glfwSetWindowUserPointer(window, &user_data);
}

Window::~Window()
{
	surface.reset();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::update() { }

void Window::handle_input(float) { glfwPollEvents(); }

auto Window::should_close() const -> bool { return glfwWindowShouldClose(window) != GLFW_FALSE; }

auto Window::get_framebuffer_size() -> std::pair<int, int>
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	return { width, height };
}

void Window::reset_resize_status() { user_data.was_resized = false; }

auto Window::was_resized() const -> bool { return user_data.was_resized; }

void Window::wait_for_minimisation()
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
}

auto Window::get_framebuffer_scale() -> std::pair<float, float>
{
	float width = 0.0F;
	float height = 0.0F;
	glfwGetWindowContentScale(window, &width, &height);
	return { width, height };
}

} // namespace Disarray::Vulkan
