#pragma once

#include <glm/glm.hpp>

#include "core/KeyCode.hpp"
#include "core/MouseCode.hpp"
#include "core/UsageBadge.hpp"
#include "core/Window.hpp"

namespace Disarray {

class App;

class Input {
public:
	static void construct(const Disarray::Window&);

	static bool button_pressed(MouseCode code);
	static bool key_pressed(KeyCode code);
	static bool button_released(MouseCode code);
	static bool key_released(KeyCode code);

	template <KeyCode... Codes> static bool all() { return (key_pressed(Codes) && ...); }
	template <KeyCode... Codes> static bool any() { return (key_pressed(Codes) || ...); }
	template <MouseCode... Codes> static bool all() { return (button_pressed(Codes) && ...); }
	template <MouseCode... Codes> static bool any() { return (button_pressed(Codes) || ...); }

	static glm::vec2 mouse_position();

	static void destruct();
};

} // namespace Disarray
