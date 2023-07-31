#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Instance;
	class Surface;
	class QueueFamilyIndex;

	class PhysicalDevice {
	public:
		static Ref<PhysicalDevice> construct(Disarray::Instance&, Disarray::Surface&);

		virtual Disarray::QueueFamilyIndex& get_queue_family_indexes() = 0;

		virtual ~PhysicalDevice() = default;
	};

} // namespace Disarray
