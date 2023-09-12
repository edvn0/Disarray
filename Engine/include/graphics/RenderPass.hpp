#pragma once

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"

namespace Disarray {

struct RenderPassProperties {
	std::string debug_name { "RenderPass" };
};

class Device;

class RenderPass : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(RenderPass, RenderPassProperties)
};

} // namespace Disarray
