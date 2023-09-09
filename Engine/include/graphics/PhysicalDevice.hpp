#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/QueueFamilyIndex.hpp"

namespace Disarray {

class Instance;
class Surface;

class PhysicalDevice : public ReferenceCountable {
	DISARRAY_OBJECT_NO_PROPS(PhysicalDevice)
public:
	static auto construct(Disarray::Instance&, Disarray::Surface&) -> Ref<Disarray::PhysicalDevice>;

	virtual auto get_queue_family_indexes() -> Disarray::QueueFamilyIndex& = 0;
	virtual auto get_queue_family_indexes() const -> const Disarray::QueueFamilyIndex& = 0;
};

} // namespace Disarray
