#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class PhysicalDevice;
	class Window;

	class Swapchain {
	public:
		static Ref<Swapchain> construct(Scope<Window>&, Ref<Device>, Ref<PhysicalDevice>);
		virtual ~Swapchain() = default;
	};

} // namespace Disarray