#pragma once

#include <fmt/core.h>

#include <cstddef>
#include <cstdint>

#include "core/Concepts.hpp"
#include "core/Types.hpp"

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

enum class Tiling { Linear, DeviceOptimal };

template <IsNumber T> struct IExtent {
	T width {};
	T height {};

	T get_size() const { return width * height; }
	float aspect_ratio() const { return static_cast<float>(width) / static_cast<float>(height); }

	bool valid() const { return width > 0 && height > 0; }

	bool operator==(const IExtent<T>& other) const { return width == other.width && height == other.height; }
	bool operator!=(const IExtent<T>& other) const { return width != other.width || height != other.height; }
};

struct Extent : public IExtent<std::uint32_t> { };
struct FloatExtent : public IExtent<float> { };

enum class ImageFormat { SRGB, RGB, SBGR, BGR, SRGB32, RGB32, Depth, DepthStencil, Uint };

} // namespace Disarray

namespace fmt {

template <> struct formatter<Disarray::SampleCount> : formatter<std::string_view> {
	auto format(Disarray::SampleCount samples, format_context& ctx) const;
};

template <> struct formatter<Disarray::ImageFormat> : formatter<std::string_view> {
	auto format(Disarray::ImageFormat format, format_context& ctx) const;
};

} // namespace fmt
