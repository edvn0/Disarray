#pragma once

#include "core/KeyCode.hpp"
#include "core/MouseCode.hpp"
#include "core/UsageBadge.hpp"
#include "core/Window.hpp"

#include <glm/glm.hpp>

namespace Disarray {

	class App;

	class Input {
	public:
		static void construct(UsageBadge<App>, Disarray::Window&);

		static bool button_pressed(MouseCode code);
		static bool key_pressed(KeyCode code);
		static bool button_released(MouseCode code);
		static bool key_released(KeyCode code);
		static glm::vec2 mouse_position();

		static void destruct();
	};

} // namespace Disarray
