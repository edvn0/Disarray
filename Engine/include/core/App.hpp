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

	class App {
	public:
		~App();
		App();
		void run();

		template <typename T, typename... Args>
		decltype(auto) add_layer(Args&&... args)
			requires (std::is_base_of_v<Layer, T> && requires (Ref<Device> dev, Ref<PhysicalDevice> phy, Scope<Window>& win, Ref<Swapchain> swap)
				{
					T(dev, phy, win, swap);
				})
		{
			return layers.emplace_back(Ref<T> { new T(device, physical_device, window, swapchain, std::forward(args)...) });
		}

		template <typename T, typename... Args>
		void add_panel(Args&&... args)
		{
			Ref<Layer> interface {nullptr};
			for (const auto& layer: layers)
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
		Ref<Swapchain> swapchain {nullptr};
		std::vector<Ref<Layer>> layers {};
	};

} // namespace Disarray
