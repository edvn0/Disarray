#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/QueueFamilyIndex.hpp"

namespace Disarray {

class Instance;
class Surface;

class PhysicalDevice : public ReferenceCountable {
	DISARRAY_OBJECT(PhysicalDevice)
public:
	static Ref<Disarray::PhysicalDevice> construct(Disarray::Instance&, Disarray::Surface&);

	virtual Disarray::QueueFamilyIndex& get_queue_family_indexes() = 0;
	virtual const Disarray::QueueFamilyIndex& get_queue_family_indexes() const = 0;
};

} // namespace Disarray
