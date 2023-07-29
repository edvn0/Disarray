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
#include "vulkan/CommandExecutor.hpp"

namespace Disarray {

	App::App()
	{
		window = Window::construct(1280, 720);
		physical_device = PhysicalDevice::construct(window->get_instance(), window->get_surface());
		device = Device::construct(physical_device);
		swapchain = Swapchain::construct(window, device, physical_device);

		renderer = Renderer::construct(device, swapchain, {});
		renderer->set_extent({.width = swapchain->get_extent().width, .height = swapchain->get_extent().height});

		initialise_allocator(device, physical_device, window->get_instance());
	}

	App::~App() { destroy_allocator(); Log::debug("App destructor called."); };

	void App::run()
	{
		for (auto& layer : layers) {
			layer->construct(renderer);
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

			float time_step = Clock::ms() - current_time;
			for (auto& layer : layers) {
				if (swapchain->needs_recreation()) layer->handle_swapchain_recreation(renderer);
				layer->update(time_step, renderer);
			}
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