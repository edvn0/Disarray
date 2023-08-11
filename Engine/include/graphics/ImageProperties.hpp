#pragma once

#include "core/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <fmt/format.h>

namespace Disarray {

	enum class SampleCount {
		One = 0x00000001,
		Two = 0x00000002,
		Four = 0x00000004,
		Eight = 0x00000008,
		Sixteen = 0x00000010,
		ThirtyTwo = 0x00000020,
		SixtyFour = 0x00000040,
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

		case Disarray::SampleCount::One:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 1), ctx);
		case Disarray::SampleCount::Two:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 2), ctx);
		case Disarray::SampleCount::Four:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 4), ctx);
		case Disarray::SampleCount::Eight:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 8), ctx);
		case Disarray::SampleCount::Sixteen:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 16), ctx);
		case Disarray::SampleCount::ThirtyTwo:
			return fmt::formatter<std::string_view>::format(fmt::format("[{}]", 32), ctx);
		case Disarray::SampleCount::SixtyFour:
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
