#pragma once

#include "core/Types.hpp"
#include "graphics/PhysicalDevice.hpp"

namespace Disarray {

	class Window;

	class Device {
	public:
		virtual ~Device() = default;
		virtual Disarray::PhysicalDevice& get_physical_device() = 0;
		virtual const Disarray::PhysicalDevice& get_physical_device() const = 0;
		static Scope<Device> construct(Disarray::Window&);
	};

} // namespace Disarray
