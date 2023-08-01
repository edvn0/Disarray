#pragma once

#include "Forward.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct RenderPassProperties {
		ImageFormat image_format { ImageFormat::SBGR };
		ImageFormat depth_format { ImageFormat::Depth };
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
		DISARRAY_MAKE_REFERENCE_COUNTABLE(RenderPass)
	public:
		virtual void force_recreation() = 0;
		virtual void recreate(bool should_clean) = 0;
		static Ref<RenderPass> construct(Device&, const RenderPassProperties&);
	};

} // namespace Disarray
