#pragma once

#include "Forward.hpp"
#include "core/DataBuffer.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct ImageProperties {
		Extent extent;
		ImageFormat format;
		DataBuffer data;
		bool should_present { false };
		std::string debug_name;
	};

	class Image {
	public:
		virtual ~Image() = default;
		virtual void force_recreation() = 0;
		virtual void recreate(bool should_clean) = 0;
		static Ref<Image> construct(Disarray::Device&, Disarray::Swapchain&, const ImageProperties&);
	};

} // namespace Disarray
