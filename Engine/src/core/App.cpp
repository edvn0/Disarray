#include "DisarrayPCH.hpp"

#include "core/AllocatorConfigurator.hpp"
#include "core/App.hpp"
#include "core/CleanupAwaiter.hpp"
#include "core/Clock.hpp"
#include "core/DebugConfigurator.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "core/ThreadPool.hpp"
#include "core/Window.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "scene/Camera.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"
#include "vulkan/CommandExecutor.hpp"

#include <memory>

namespace Disarray {

App::App(const Disarray::ApplicationProperties& props)
{
	window = Window::construct({ .width = props.width, .height = props.height, .name = props.name, .is_fullscreen = props.is_fullscreen });
	device = Device::construct(*window);
	window->register_event_handler(*this);

	initialise_debug_applications(*device);
	initialise_allocator(*device, window->get_instance());
	swapchain = Swapchain::construct(*window, *device);

	Input::construct({}, *window);
}

void App::on_event(Event& event)
{
	for (const auto& layer : layers) {
		if (event.handled)
			return;

		layer->on_event(event);
	}
}

App::~App() { destroy_allocator(); }

void App::run()
{
	on_attach();

	ThreadPool pool { 2 };

	auto constructed_renderer = Renderer::construct(*device, *swapchain, {});

	auto& l = add_layer<UI::InterfaceLayer>();
	auto ui_layer = std::dynamic_pointer_cast<UI::InterfaceLayer>(l);

	UI::DescriptorCache::initialise();

	auto& renderer = *constructed_renderer;

	for (auto& layer : layers) {
		layer->construct(*this, renderer, pool);
	}

	static float current_time = Clock::ms();
	while (!window->should_close()) {
		window->update();

		if (!could_prepare_frame(renderer))
			continue;

		const auto needs_recreation = swapchain->needs_recreation();
		float time_step = Clock::ms() - current_time;

		for (auto& layer : layers) {
			if (needs_recreation)
				layer->handle_swapchain_recreation(*swapchain);
			layer->update(time_step);
			layer->render(renderer);
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

	UI::DescriptorCache::destruct();

	for (auto& layer : layers) {
		layer->destruct();
	}
	layers.clear();

	on_detach();
}

bool App::could_prepare_frame(Renderer& renderer)
{
	const auto could_prepare = swapchain->prepare_frame();
	if (could_prepare)
		return true;

	renderer.force_recreation();
	for (auto& layer : layers) {
		layer->handle_swapchain_recreation(*swapchain);
	}
	return false;
}

} // namespace Disarray
