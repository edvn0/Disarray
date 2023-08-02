#pragma once

#include "core/Layer.hpp"
#include "core/Types.hpp"
#include "graphics/PipelineCache.hpp"
#include "ui/InterfaceLayer.hpp"

#include <memory>
#include <vector>

namespace Disarray {

	class Window;
	class Device;
	class Swapchain;
	class PhysicalDevice;
	class Renderer;

	struct ApplicationProperties {
		std::uint32_t width { 0 };
		std::uint32_t height { 0 };
		std::string name {};
	};

	class App {
	public:
		virtual ~App();
		App(const ApplicationProperties&);
		void run();

		virtual void on_attach() = 0;

		template <typename T, typename... Args>
		decltype(auto) add_layer(Args&&... args)
			requires(std::is_base_of_v<Layer, T>
				&& requires(Disarray::Device & dev, Disarray::Window& win, Disarray::Swapchain& swap) { T(dev, win, swap); })
		{
			return layers.emplace_back(std::shared_ptr<T> { new T(*device, *window, *swapchain, std::forward<Args>(args)...) });
		}

		template <typename T, typename... Args> void add_panel(Args&&... args)
		{
			std::shared_ptr<Layer> interface {
				nullptr
			};
			for (const auto& layer : layers)
				if (layer->is_interface_layer()) {
					interface = layer;
					break;
				}

			auto interface_layer = std::dynamic_pointer_cast<UI::InterfaceLayer>(interface);
			interface_layer->template add_panel<T>(std::forward<Args>(args)...);
		}

	private:
		Scope<Window> window { nullptr };
		Scope<PhysicalDevice> physical_device { nullptr };
		Scope<Device> device { nullptr };
		Scope<Swapchain> swapchain { nullptr };
		std::vector<std::shared_ptr<Layer>> layers {};
	};

	extern std::unique_ptr<Disarray::App> create_application(const Disarray::ApplicationProperties&);

} // namespace Disarray
