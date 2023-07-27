#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Surface;
	class PhysicalDevice;

	class Device {
	public:
		static Scope<Device> construct(Ref<PhysicalDevice> device, Ref<Surface> surface);
		virtual ~Device() = default;
	};

} // namespace Disarray
