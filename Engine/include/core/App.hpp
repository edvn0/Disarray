#pragma once

#include "core/Layer.hpp"
#include "core/Types.hpp"

#include <vector>

namespace Disarray {

	class Window;
	class Device;
	class Swapchain;
	class PhysicalDevice;

	class App {
	public:
		~App();
		App();
		void run();

		template <typename T, typename... Args>
		void add_layer(Args&&... args)
			requires std::is_base_of_v<Layer, T>
		{
			layers.emplace_back(make_ref<T>(std::forward(args)...));
		}

	private:
		Scope<Window> window { nullptr };
		Ref<PhysicalDevice> physical_device { nullptr };
		Ref<Device> device { nullptr };
		Ref<Swapchain> swapchain {nullptr};
		std::vector<Ref<Layer>> layers {};
	};

} // namespace Disarray
