#include "DisarrayPCH.hpp"

#include "core/App.hpp"

#include "core/AllocatorConfigurator.hpp"
#include "core/CleanupAwaiter.hpp"
#include "core/Clock.hpp"
#include "core/DebugConfigurator.hpp"
#include "core/Log.hpp"
#include "core/ThreadPool.hpp"
#include "core/Window.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"
#include "vulkan/CommandExecutor.hpp"

#include <core/Input.hpp>
#include <memory>
#include <scene/Camera.hpp>

namespace Disarray {

	App::App(const Disarray::ApplicationProperties& props)
	{
		window = Window::construct({ .width = props.width, .height = props.height, .name = props.name, .is_fullscreen = props.is_fullscreen });
		device = Device::construct(*window);

		initialise_debug_applications(*device);
		initialise_allocator(*device, window->get_instance());
		swapchain = Swapchain::construct(*window, *device);

		Input::construct({}, *window);
	}

	App::~App() { destroy_allocator(); }

	void App::run()
	{
		on_attach();

		ThreadPool pool { 2 };

		auto constructed_renderer = Renderer::construct(*device, *swapchain, {});
		constructed_renderer->on_resize();

		auto& l = add_layer<UI::InterfaceLayer>();
		auto ui_layer = std::dynamic_pointer_cast<UI::InterfaceLayer>(l);

		EditorCamera camera { 73.f, static_cast<float>(swapchain->get_extent().width), static_cast<float>(swapchain->get_extent().height), 0.1f,
			1000.f, nullptr };

		UI::DescriptorCache::initialise();

		auto& renderer = *constructed_renderer;

		for (auto& layer : layers) {
			layer->construct(*this, renderer, pool);
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

			renderer.begin_frame({}, camera);
			const auto needs_recreation = swapchain->needs_recreation();
			float time_step = Clock::ms() - current_time;
			camera.on_update(time_step);
			if (needs_recreation)
				camera.set_viewport_size(swapchain->get_extent());
			for (auto& layer : layers) {
				if (needs_recreation)
					layer->handle_swapchain_recreation(renderer);
				layer->update(time_step, renderer);
			}
			statistics.cpu_time = time_step;
			ui_layer->begin();
			for (auto& layer : layers) {
				layer->interface();
			}
			ui_layer->end();
			renderer.end_frame({});

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

} // namespace Disarray
