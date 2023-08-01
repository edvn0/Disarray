#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/QueueFamilyIndex.hpp"

namespace Disarray {

	class Instance;
	class Surface;

	class PhysicalDevice : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(PhysicalDevice)
	public:
		static Ref<PhysicalDevice> construct(Disarray::Instance&, Disarray::Surface&);

		virtual Disarray::QueueFamilyIndex& get_queue_family_indexes() = 0;
	};

} // namespace Disarray
