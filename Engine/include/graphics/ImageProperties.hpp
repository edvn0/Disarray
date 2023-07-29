#pragma once

#include <cstdint>
#include <cstddef>

namespace Disarray {

	struct Extent {
		std::uint32_t width {};
		std::uint32_t height {};

		std::size_t get_size() const { return width*height;}
	};

	enum class ImageFormat {
		SRGB,
		RGB,
		SBGR,
		BGR,
		Depth,
		DepthStencil
	};

}