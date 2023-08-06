#pragma once

#include "Forward.hpp"
#include "core/DataBuffer.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct ImageProperties {
		Extent extent;
		ImageFormat format;
		DataBuffer data;
		bool should_present { false };
		SampleCount samples { SampleCount::ONE };
		std::string debug_name;
	};

	class Image : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(Image)
	public:
		virtual void force_recreation() = 0;
		virtual void recreate(bool should_clean) = 0;
		static Ref<Image> construct(Disarray::Device&, Disarray::Swapchain&, const ImageProperties&);
	};

} // namespace Disarray
