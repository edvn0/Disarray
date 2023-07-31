#include "core/Window.hpp"

#include "core/App.hpp"
#include "vulkan/Window.hpp"

namespace Disarray {

	Window::Window(const Disarray::ApplicationProperties& properties):props(properties) {}

	Scope<Window> Window::construct(const Disarray::ApplicationProperties& properties) { return make_scope<Vulkan::Window>(properties); }

	const Disarray::ApplicationProperties& Window::get_properties() { return props; }

} // namespace Disarray
