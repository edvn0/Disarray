#pragma once

#include "core/Layer.hpp"
#include "core/Types.hpp"
#include "graphics/PipelineCache.hpp"
#include "ui/InterfaceLayer.hpp"

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
			requires(std::is_base_of_v<Layer, T> && requires(Device& dev, PhysicalDevice& phy, Window& win, Swapchain& swap) { T(dev, win, swap); })
		{
			return layers.emplace_back(Ref<T> { new T(*device, *window, *swapchain, std::forward(args)...) });
		}

		template <typename T, typename... Args> void add_panel(Args&&... args)
		{
			Ref<Layer> interface {
				nullptr
			};
			for (const auto& layer : layers)
				if (layer->is_interface_layer()) {
					interface = layer;
					break;
				}

			auto interface_layer = cast_to<UI::InterfaceLayer>(interface);
			interface_layer->add_panel<T>(std::forward<Args>(args)...);
		}

	private:
		Scope<Window> window { nullptr };
		Ref<PhysicalDevice> physical_device { nullptr };
		Ref<Device> device { nullptr };
		Ref<Swapchain> swapchain { nullptr };
		std::vector<Ref<Layer>> layers {};
	};

	extern std::unique_ptr<Disarray::App> create_application(const Disarray::ApplicationProperties&);

} // namespace Disarray
