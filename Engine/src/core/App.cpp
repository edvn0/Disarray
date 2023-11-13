#include "DisarrayPCH.hpp"

#include <filesystem>
#include <memory>
#include <thread>

#include "core/AllocatorConfigurator.hpp"
#include "core/App.hpp"
#include "core/Clock.hpp"
#include "core/DebugConfigurator.hpp"
#include "core/Formatters.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "core/ThreadPool.hpp"
#include "core/Window.hpp"
#include "graphics/Swapchain.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"

namespace Disarray {

App::App(const Disarray::ApplicationProperties& props)
{
	const auto& path = props.working_directory;
	std::filesystem::current_path(path);

	window = Window::construct({
		.width = props.width,
		.height = props.height,
		.name = props.name,
		.is_fullscreen = props.is_fullscreen,
		.use_validation_layers = props.use_validation_layers,
	});
	device = Device::construct(*window);
	window->register_event_handler(*this);

	initialise_debug_applications(*device);
	initialise_allocator(*device, window->get_instance());
	swapchain = Swapchain::construct(*window, *device);
}

void App::on_event(Event& event)
{
	for (const auto& layer : layers) {
		if (event.handled) {
			return;
		}

		layer->on_event(event);
	}
}

auto AppDeleter::operator()(Disarray::App* ptr) -> void
{
	Log::info("App", "{}", "Successfully exited application.");
	delete ptr;
}

App::~App()
{
	destroy_debug_applications();
	destroy_allocator();
}

void App::run()
{
	on_attach();

	auto ui_layer = add_layer<UI::InterfaceLayer>();

	UI::InterfaceCaches::initialise();

	for (auto& layer : layers) {
		layer->construct(*this);
	}

	static constexpr float minimum_time_step = 1000.0F * 0.0333F;
	static constexpr auto minimum_hertz = 1000.0F / minimum_time_step;
	static auto current_time = Clock::ms();
	static float step = minimum_time_step;
	while (!window->should_close()) {
		const auto could_prepare = could_prepare_frame();

#ifdef DISARRAY_VSYNC
		if (step < 16.0) {
			const auto sleep_time = std::chrono::duration<double, std::milli>(16.0 - step);
			std::this_thread::sleep_for(sleep_time);
		}
#endif

		window->handle_input(step);
		update_layers(step, could_prepare);
		render_layers();
		Renderer::execute_queue();
		statistics.cpu_time = step;
		render_ui(ui_layer);

		swapchain->reset_recreation_status();

		auto begin_present_time = Clock::ns();
		swapchain->present();
		statistics.presentation_time = Clock::ns() - begin_present_time;

		window->update();
		statistics.frame_time = Clock::ms() - current_time;
		current_time = Clock::ms();

		step = glm::min<float>(statistics.frame_time, minimum_time_step);
	}

	wait_for_idle(*device);

	UI::InterfaceCaches::destruct();
	for (auto& layer : layers) {
		layer->destruct();
	}
	layers.clear();

	on_detach();
}

void App::update_layers(float time_step, bool could_prepare)
{
	for (auto& layer : layers) {
		if (swapchain->needs_recreation() || !could_prepare) {
			layer->handle_swapchain_recreation(*swapchain);
		}
		layer->update(time_step);
	}
}

void App::render_layers()
{
	for (auto& layer : layers) {
		layer->render();
	}
}

void App::render_ui(const std::shared_ptr<Disarray::UI::InterfaceLayer>& ui_layer)
{
	ui_layer->begin();
	for (auto& layer : layers) {
		layer->interface();
	}
	ui_layer->end();
}

auto App::could_prepare_frame() -> bool
{
	const auto could_prepare = swapchain->prepare_frame();
	if (could_prepare) {
		return true;
	}

	for (auto& layer : layers) {
		layer->handle_swapchain_recreation(*swapchain);
	}
	return false;
}

} // namespace Disarray
