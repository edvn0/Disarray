#include "core/Window.hpp"

#include "vulkan/Window.hpp"

namespace Disarray {

	Window::Window(std::uint32_t w, std::uint32_t h)
		: width(w)
		, height(h) {};

	Scope<Window> Window::construct(std::uint32_t w, std::uint32_t h) { return make_scope<Vulkan::Window>(w, h); }

} // namespace Disarray
