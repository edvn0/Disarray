#pragma once

#include "core/Window.hpp"
#include "core/events/Event.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/Swapchain.hpp"

extern "C" {
struct GLFWwindow;
struct GLFWmonitor;
struct GLFWvidmode;
}

namespace Disarray::Vulkan {

using EventCallback = std::function<void(Disarray::Event&)>;

static constexpr auto default_event_callback = [](Disarray::Event&) mutable {};

struct UserData {
	bool was_resized { false };
	bool fullscreen { false };
	std::uint32_t width { 0 };
	std::uint32_t height { 0 };
	EventCallback callback { default_event_callback };
	int pos_x { 0 };
	int pos_y { 0 };
	const GLFWvidmode* mode { nullptr };
	GLFWmonitor* monitor { nullptr };
};

class Window : public Disarray::Window {
public:
	Window(const Disarray::WindowProperties&);
	~Window() override;

	bool should_close() const override;
	void update() override;
	Disarray::Surface& get_surface() override { return *surface; };
	Disarray::Instance& get_instance() override { return *instance; };

	void register_event_handler(App&) override;

	void reset_resize_status() override;
	bool was_resized() const override;

	void wait_for_minimisation() override;

	void* native() override { return window; }
	void* native() const override { return window; }

	std::pair<int, int> get_framebuffer_size() override;
	std::pair<float, float> get_framebuffer_scale() override;

private:
	UserData user_data;

	GLFWwindow* window { nullptr };
	Scope<Instance> instance { nullptr };
	Scope<Surface> surface { nullptr };
};

} // namespace Disarray::Vulkan
