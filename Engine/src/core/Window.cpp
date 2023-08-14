#include "DisarrayPCH.hpp"

#include "core/App.hpp"
#include "core/Window.hpp"
#include "vulkan/Window.hpp"

namespace Disarray {

Window::Window(const WindowProperties& properties)
	: props(properties)
{
}

Scope<Window> Window::construct(const WindowProperties& properties) { return make_scope<Vulkan::Window>(properties); }

const WindowProperties& Window::get_properties() { return props; }

} // namespace Disarray
