#include "DisarrayPCH.hpp"

#include "core/Window.hpp"

#include "core/App.hpp"
#include "vulkan/Window.hpp"

namespace Disarray {

Window::Window(const WindowProperties& properties)
	: props(properties)
{
}

auto Window::construct(const WindowProperties& properties) -> Scope<Window> { return make_scope<Vulkan::Window>(properties); }

auto Window::get_properties() -> const WindowProperties& { return props; }

} // namespace Disarray
