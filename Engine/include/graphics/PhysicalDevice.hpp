#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Instance;
	class Surface;

	class PhysicalDevice {
	public:
		static Scope<PhysicalDevice> construct(Ref<Instance> instance, Ref<Surface>);
		virtual ~PhysicalDevice() = default;
	};

} // namespace Disarray
