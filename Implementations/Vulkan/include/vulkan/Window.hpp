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

	[[nodiscard]] auto should_close() const -> bool override;
	void update() override;
	void handle_input(float time_step) override;
	auto get_surface() -> Disarray::Surface& override { return *surface; };
	auto get_instance() -> Disarray::Instance& override { return *instance; };

	void register_event_handler(App&) override;

	void reset_resize_status() override;
	[[nodiscard]] auto was_resized() const -> bool override;

	void wait_for_minimisation() override;

	auto native() -> void* override { return window; }
	[[nodiscard]] auto native() const -> void* override { return window; }

	auto get_framebuffer_size() -> std::pair<int, int> override;
	auto get_framebuffer_scale() -> std::pair<float, float> override;

private:
	UserData user_data;

	GLFWwindow* window { nullptr };
	Scope<Instance> instance { nullptr };
	Scope<Surface> surface { nullptr };
};

} // namespace Disarray::Vulkan
