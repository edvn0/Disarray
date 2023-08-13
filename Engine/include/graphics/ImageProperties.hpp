#pragma once

#include "core/Types.hpp"

#include <cstddef>
#include <cstdint>
#include <fmt/core.h>

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
	auto format(Disarray::SampleCount samples, format_context& ctx);
};

template <> struct fmt::formatter<Disarray::ImageFormat> : fmt::formatter<std::string_view> {
	auto format(Disarray::ImageFormat format, format_context& ctx);
};
