#pragma once

#include "core/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <fmt/format.h>

namespace Disarray {

	enum class SampleCount {
		ONE = 0x00000001,
		TWO = 0x00000002,
		FOUR = 0x00000004,
		EIGHT = 0x00000008,
		SIXTEEN = 0x00000010,
		THIRTY_TWO = 0x00000020,
		SIXTY_FOUR = 0x00000040,
	};

	struct Extent {
		std::uint32_t width {};
		std::uint32_t height {};

		std::uint32_t get_size() const { return width * height; }
	};

	enum class ImageFormat { SRGB, RGB, SBGR, BGR, Depth, DepthStencil };

} // namespace Disarray

template <> struct fmt::formatter<Disarray::SampleCount> : fmt::formatter<std::string_view> {
	auto format(Disarray::SampleCount samples, format_context& ctx)
	{
		switch (samples) {

		case Disarray::SampleCount::ONE:
			return formatter<std::string_view>::format(fmt::format("[{}]", 1), ctx);
		case Disarray::SampleCount::TWO:
			return formatter<std::string_view>::format(fmt::format("[{}]", 2), ctx);
		case Disarray::SampleCount::FOUR:
			return formatter<std::string_view>::format(fmt::format("[{}]", 4), ctx);
		case Disarray::SampleCount::EIGHT:
			return formatter<std::string_view>::format(fmt::format("[{}]", 8), ctx);
		case Disarray::SampleCount::SIXTEEN:
			return formatter<std::string_view>::format(fmt::format("[{}]", 16), ctx);
		case Disarray::SampleCount::THIRTY_TWO:
			return formatter<std::string_view>::format(fmt::format("[{}]", 32), ctx);
		case Disarray::SampleCount::SIXTY_FOUR:
			return formatter<std::string_view>::format(fmt::format("[{}]", 64), ctx);
		default:
			Disarray::unreachable();
		}
	}
};
