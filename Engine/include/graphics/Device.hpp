#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class PhysicalDevice;
	class Window;

	class Device {
	public:
		static Scope<Device> construct(Disarray::Window&);
		virtual PhysicalDevice& get_physical_device() = 0;
		virtual ~Device() = default;
	};

} // namespace Disarray
