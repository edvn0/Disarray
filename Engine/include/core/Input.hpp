#pragma once

#include "core/KeyCode.hpp"
#include "core/MouseCode.hpp"
#include "core/UsageBadge.hpp"
#include "core/Window.hpp"

namespace Disarray {

	class App;

	class Input {
	public:
		static void construct(UsageBadge<App>, Disarray::Window&);

		static bool button_pressed(MouseCode code);
		static bool button_pressed(KeyCode code);

		static void destruct();
	};

} // namespace Disarray
