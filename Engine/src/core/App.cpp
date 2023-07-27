#include "core/app.hpp"

#include "core/Clock.hpp"
#include "core/Log.hpp"
#include "core/Window.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

	App::App()
	{
		window = Window::construct(1280, 720);
		physical_device = PhysicalDevice::construct(window->get_instance(), window->get_surface());
		device = Device::construct(physical_device, window->get_surface());
		swapchain = Swapchain::construct(window, device, physical_device);
	}

	App::~App() { Log::debug("App destructor called."); };

	void App::run()
	{
		for (auto& layer : layers) {
			layer->construct();
		}

		while (!window->should_close()) {
			window->update();

			static float current_time = Clock::ms();
			for (auto& layer : layers) {
				float time_step = Clock::ms() - current_time;
				layer->update(time_step);
				current_time = Clock::ms();
			}
		}

		for (auto& layer : layers) {
			layer->destruct();
		}
	}

} // namespace Disarray