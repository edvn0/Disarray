#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Instance;
	class Surface;
	class QueueFamilyIndex;

	class PhysicalDevice {
	public:
		static Scope<PhysicalDevice> construct(Ref<Instance> instance, Ref<Surface>);

		virtual Ref<Disarray::QueueFamilyIndex> get_queue_family_indexes() = 0;

		virtual ~PhysicalDevice() = default;
	};

} // namespace Disarray
