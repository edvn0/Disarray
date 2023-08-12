#pragma once

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct RenderPassProperties {
		std::string debug_name { "RenderPass" };
	};

	class Device;

	class RenderPass : public ReferenceCountable {
		DISARRAY_OBJECT(RenderPass)
	public:
		static Ref<RenderPass> construct(const Device&, const RenderPassProperties& = {});
	};

} // namespace Disarray
