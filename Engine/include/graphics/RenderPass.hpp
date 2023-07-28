#pragma once

#include "graphics/ImageProperties.hpp"
#include "core/Types.hpp"

namespace Disarray {

	struct RenderPassProperties {
		ImageFormat image_format { ImageFormat::SRGB };
		bool has_depth {false};
	};

	class Device;

	class RenderPass {
	public:
		virtual ~RenderPass() = default;
		virtual void force_recreation() = 0;
		static Ref<RenderPass> construct(Ref<Device>, const RenderPassProperties&);
	};

}