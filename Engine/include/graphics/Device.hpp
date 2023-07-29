#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class QueueFamilyIndex;
	class PhysicalDevice;

	class Device {
	public:
		static Scope<Device> construct(Ref<Disarray::PhysicalDevice>);
		virtual ~Device() = default;
	};

} // namespace Disarray
