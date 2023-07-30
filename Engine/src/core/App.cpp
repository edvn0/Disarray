#include "core/app.hpp"

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

	App::App()
	{
		window = Window::construct(1280, 720);
		physical_device = PhysicalDevice::construct(window->get_instance(), window->get_surface());
		device = Device::construct(physical_device);
		swapchain = Swapchain::construct(window, device, physical_device);

		initialise_allocator(device, physical_device, window->get_instance());
	}

	App::~App() { destroy_allocator(); Log::debug("App destructor called."); };

	void App::run()
	{
		auto constructed_renderer = Renderer::construct(device, swapchain, physical_device, {});
		constructed_renderer->set_extent({.width = swapchain->get_extent().width, .height = swapchain->get_extent().height});
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
				for (auto& layer : layers) {
					layer->handle_swapchain_recreation(renderer);
				}
				continue;
			}
			renderer.begin_frame({});
			ui_layer->begin();
			float time_step = Clock::ms() - current_time;
			for (auto& layer : layers) {
				if (swapchain->needs_recreation()) layer->handle_swapchain_recreation(renderer);
				layer->update(time_step, renderer);
			}
			ui_layer->end(renderer);
			renderer.end_frame({});
			swapchain->reset_recreation_status();
			current_time = Clock::ms();
			swapchain->present();
		}

		wait_for_cleanup(device);

		for (auto& layer : layers) {
			layer->destruct();
		}
		layers.clear();
	}

} // namespace Disarray