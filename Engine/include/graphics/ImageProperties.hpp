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

	template <class T> struct IExtent {
		T width {};
		T height {};

		T get_size() const { return width * height; }
		float aspect_ratio() const { return static_cast<float>(width) / static_cast<float>(height); }

		bool operator==(const IExtent<T>& other) const { return width == other.width && height == other.height; }
		bool operator!=(const IExtent<T>& other) const { return width != other.width || height != other.height; }
	};

	struct Extent : public IExtent<std::uint32_t> { };
	struct FloatExtent : public IExtent<float> { };

	enum class ImageFormat { SRGB, RGB, SBGR, BGR, SRGB32, RGB32, Depth, DepthStencil, Uint };

} // namespace Disarray

template <> struct fmt::formatter<Disarray::SampleCount> : fmt::formatter<std::string_view> {
	auto format(Disarray::SampleCount samples, format_context& ctx)
	{
		switch (samples) {

		case Disarray::SampleCount::ONE:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 1), ctx);
		case Disarray::SampleCount::TWO:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 2), ctx);
		case Disarray::SampleCount::FOUR:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 4), ctx);
		case Disarray::SampleCount::EIGHT:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 8), ctx);
		case Disarray::SampleCount::SIXTEEN:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 16), ctx);
		case Disarray::SampleCount::THIRTY_TWO:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 32), ctx);
		case Disarray::SampleCount::SIXTY_FOUR:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 64), ctx);
		default:
			Disarray::unreachable();
		}
	}
};

template <> struct fmt::formatter<Disarray::ImageFormat> : fmt::formatter<std::string_view> {
	auto format(Disarray::ImageFormat format, format_context& ctx)
	{
		switch (format) {
		default:
			Disarray::unreachable();
		case Disarray::ImageFormat::SRGB:
			return formatter<std::string_view>::format(fmt::format("[{}]", "SRGB"), ctx);
		case Disarray::ImageFormat::RGB:
			return formatter<std::string_view>::format(fmt::format("[{}]", "RGB"), ctx);
		case Disarray::ImageFormat::SBGR:
			return formatter<std::string_view>::format(fmt::format("[{}]", "SBGR"), ctx);
		case Disarray::ImageFormat::BGR:
			return formatter<std::string_view>::format(fmt::format("[{}]", "BGR"), ctx);
		case Disarray::ImageFormat::Depth:
			return formatter<std::string_view>::format(fmt::format("[{}]", "Depth"), ctx);
		case Disarray::ImageFormat::Uint:
			return formatter<std::string_view>::format(fmt::format("[{}]", "Uint"), ctx);
		case Disarray::ImageFormat::DepthStencil:
			return formatter<std::string_view>::format(fmt::format("[{}]", "DepthStencil"), ctx);
		}
	}
};
