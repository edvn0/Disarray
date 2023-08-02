#pragma once

#include "core/Types.hpp"
#include "graphics/PhysicalDevice.hpp"

namespace Disarray {

	class Window;

	class Device {
	public:
		static Scope<Device> construct(Disarray::Window&);
		virtual Disarray::PhysicalDevice& get_physical_device() = 0;
		virtual ~Device() = default;
	};

} // namespace Disarray
