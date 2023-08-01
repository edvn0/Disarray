#include "core/App.hpp"

#include "core/AllocatorConfigurator.hpp"
#include "core/CleanupAwaiter.hpp"
#include "core/Clock.hpp"
#include "core/Log.hpp"
#include "core/Window.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "ui/InterfaceLayer.hpp"
#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

	App::App(const Disarray::ApplicationProperties& props)
	{
		window = Window::construct(props);
		device = Device::construct(*window);

		initialise_allocator(*device, window->get_instance());
		swapchain = Swapchain::construct(*window, *device);
	}

	App::~App()
	{
		swapchain.reset();
		destroy_allocator();
	};

	void App::run()
	{
		on_attach();

		auto constructed_renderer = Renderer::construct(*device, *swapchain, {});
		constructed_renderer->set_extent({ .width = swapchain->get_extent().width, .height = swapchain->get_extent().height });
		auto l = add_layer<UI::InterfaceLayer>();
		auto ui_layer = cast_to<UI::InterfaceLayer>(l);

		auto& renderer = *constructed_renderer;

		for (auto& layer : layers) {
			layer->construct(*this, renderer);
		}

		static float current_time = Clock::ms();
		while (!window->should_close()) {
			window->update();

			if (!swapchain->prepare_frame()) {
				renderer.force_recreation();
				for (auto& layer : layers) {
					layer->handle_swapchain_recreation(renderer);
				}
				continue;
			}

			renderer.begin_frame({});
			const auto needs_recreation = swapchain->needs_recreation();
			float time_step = Clock::ms() - current_time;
			for (auto& layer : layers) {
				if (needs_recreation)
					layer->handle_swapchain_recreation(renderer);
				layer->update(time_step, renderer);
			}
			ui_layer->begin();
			for (auto& layer : layers) {
				layer->interface();
			}
			ui_layer->end();
			renderer.end_frame({});

			swapchain->reset_recreation_status();
			swapchain->present();
			current_time = Clock::ms();
		}

		wait_for_cleanup(*device);

		for (auto& layer : layers) {
			layer->destruct();
		}
		layers.clear();
	}

} // namespace Disarray
