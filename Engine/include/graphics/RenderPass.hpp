#pragma once

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct RenderPassProperties {
		ImageFormat image_format { ImageFormat::SBGR };
		ImageFormat depth_format { ImageFormat::Depth };
		SampleCount samples { SampleCount::ONE };
		bool load_colour { false };
		bool keep_colour { true };
		bool load_depth { false };
		bool keep_depth { true };
		bool has_depth { true };
		bool should_present { false };
		std::string debug_name { "Unknown" };
	};

	class Device;

	class RenderPass : public ReferenceCountable {
		DISARRAY_OBJECT(RenderPass)
	public:
		static Ref<RenderPass> construct(Device&, const RenderPassProperties&);
	};

} // namespace Disarray
