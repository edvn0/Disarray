#pragma once

#include <utility>

#include "graphics/Instance.hpp"
#include "graphics/Surface.hpp"

namespace Disarray {

struct WindowProperties {
	std::uint32_t width { 0 };
	std::uint32_t height { 0 };
	std::string name {};
	bool is_fullscreen { false };
	bool use_validation_layers { true };
};

class App;

class Window {
public:
	virtual ~Window() = default;

	auto get_properties() -> const WindowProperties&;

	[[nodiscard]] virtual auto should_close() const -> bool = 0;
	virtual void update() = 0;
	virtual void handle_input(float time_step) = 0;
	virtual void handle_input(double time_step) { handle_input(static_cast<float>(time_step)); };
	virtual auto get_surface() -> Surface& = 0;
	virtual auto get_instance() -> Instance& = 0;

	virtual void register_event_handler(App&) = 0;

	[[nodiscard]] virtual auto was_resized() const -> bool = 0;
	virtual void reset_resize_status() = 0;

	virtual void wait_for_minimisation() = 0;

	virtual auto native() -> void* = 0;
	[[nodiscard]] virtual auto native() const -> void* = 0;

	virtual auto get_framebuffer_size() -> std::pair<int, int> = 0;
	virtual auto get_framebuffer_scale() -> std::pair<float, float> = 0;

protected:
	Window(const WindowProperties&);

private:
	WindowProperties props;

public:
	static auto construct(const WindowProperties&) -> Scope<Disarray::Window>;
};

} // namespace Disarray
