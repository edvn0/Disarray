#pragma once

#include <glm/glm.hpp>

#include "core/KeyCode.hpp"
#include "core/MouseCode.hpp"
#include "core/Window.hpp"

namespace Disarray {

class App;

class Input {
public:
	static void construct(const Disarray::Window&);

	static auto button_pressed(MouseCode code) -> bool;
	static auto key_pressed(KeyCode code) -> bool;
	static auto button_released(MouseCode code) -> bool;
	static auto key_released(KeyCode code) -> bool;

	template <KeyCode... Codes> static auto all() -> bool { return (key_pressed(Codes) && ...); }
	template <KeyCode... Codes> static auto any() -> bool { return (key_pressed(Codes) || ...); }
	template <MouseCode... Codes> static auto all() -> bool { return (button_pressed(Codes) && ...); }
	template <MouseCode... Codes> static auto any() -> bool { return (button_pressed(Codes) || ...); }

	static auto mouse_position() -> glm::vec2;

	static void destruct();

private:
	struct WindowData;
	struct WindowDataDeleter {
		auto operator()(WindowData*) -> void;
	};
	inline static Scope<WindowData, WindowDataDeleter> window_data;
};

} // namespace Disarray
