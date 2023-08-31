#include "DisarrayPCH.hpp"

#include "core/App.hpp"

#include <filesystem>
#include <memory>

#include "core/AllocatorConfigurator.hpp"
#include "core/Clock.hpp"
#include "core/DebugConfigurator.hpp"
#include "core/Formatters.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "core/ThreadPool.hpp"
#include "core/Window.hpp"
#include "graphics/Renderer.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"

namespace Disarray {

App::App(const Disarray::ApplicationProperties& props)
{
	Log::info("App", "Working directory configured to: {}", props.working_directory);
	std::filesystem::current_path(props.working_directory);

	window = Window::construct({ .width = props.width, .height = props.height, .name = props.name, .is_fullscreen = props.is_fullscreen });
	device = Device::construct(*window);
	window->register_event_handler(*this);

	initialise_debug_applications(*device);
	initialise_allocator(*device, window->get_instance());
	swapchain = Swapchain::construct(*window, *device);
}

void App::on_event(Event& event)
{
	for (const auto& layer : layers) {
		if (event.handled)
			return;

		layer->on_event(event);
	}
}

App::~App()
{
	destroy_allocator();
	std::flush(std::cout);
}

void App::run()
{
	on_attach();

	ThreadPool pool { 8 };

	auto ui_layer = add_layer<UI::InterfaceLayer>();

	UI::InterfaceCaches::initialise();

	for (auto& layer : layers) {
		layer->construct(*this, pool);
	}

	static float current_time = Clock::ms();
	while (!window->should_close()) {
		window->update();

		if (!could_prepare_frame())
			continue;

		const auto needs_recreation = swapchain->needs_recreation();
		float time_step = Clock::ms() - current_time;

		for (auto& layer : layers) {
			if (needs_recreation) {
				layer->handle_swapchain_recreation(*swapchain);
			}
			layer->update(time_step);
			layer->render();
		}
		statistics.cpu_time = time_step;

		ui_layer->begin();
		for (auto& layer : layers) {
			layer->interface();
		}
		ui_layer->end();

		auto begin_present_time = Clock::ns();
		swapchain->reset_recreation_status();
		swapchain->present();
		statistics.presentation_time = Clock::ns() - begin_present_time;
		statistics.frame_time = Clock::ms() - current_time;
		current_time = Clock::ms();
	}

	wait_for_cleanup(*device);

	UI::InterfaceCaches::destruct();
	for (auto& layer : layers) {
		layer->destruct();
	}
	layers.clear();

	on_detach();
}

auto App::could_prepare_frame() -> bool
{
	const auto could_prepare = swapchain->prepare_frame();
	if (could_prepare)
		return true;

	for (auto& layer : layers) {
		layer->handle_swapchain_recreation(*swapchain);
	}
	return false;
}

} // namespace Disarray
