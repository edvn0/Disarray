#pragma once

#include "Forward.hpp"
#include "core/DataBuffer.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct ImageProperties {
		Extent extent;
		ImageFormat format;
		DataBuffer data;
	};

	class Image {
	public:
		virtual ~Image() = default;
		virtual void force_recreation() = 0;
		virtual void recreate(bool should_clean) = 0;
		static Ref<Image> construct(Ref<Device>, Ref<Swapchain>, Ref<PhysicalDevice>, const ImageProperties&);
	};

}
