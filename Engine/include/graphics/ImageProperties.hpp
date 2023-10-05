#pragma once

#include <cstddef>
#include <cstdint>

#include "core/Concepts.hpp"
#include "core/Types.hpp"

namespace Disarray {

enum class BorderColour : std::uint8_t {
	FloatTransparentBlack = 0,
	IntTransparentBlack = 1,
	FloatOpaqueBlack = 2,
	IntOpaqueBlack = 3,
	FloatOpaqueWhite = 4,
	IntOpaqueWhite = 5,
};

enum class SamplerMode : std::uint8_t {
	Repeat = 0,
	MirroredRepeat = 1,
	ClampToEdge = 2,
	ClampToBorder = 3,
	MirrorClampToEdge = 4,
};

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

	[[nodiscard]] auto get_size() const -> T { return width * height; }
	[[nodiscard]] auto aspect_ratio() const -> float { return static_cast<float>(width) / static_cast<float>(height); }

	[[nodiscard]] auto valid() const -> bool { return width > 0 && height > 0; }

	auto operator==(const IExtent<T>& other) const -> bool { return width == other.width && height == other.height; }
	auto operator!=(const IExtent<T>& other) const -> bool { return width != other.width || height != other.height; }

	auto to_string() -> std::string { return fmt::format("{}:{}", width, height); }

	template <IsNumber Other>
		requires(!std::is_same_v<T, Other>)
	auto as() const -> IExtent<Other>
	{
		return { .width = static_cast<Other>(width), .height = static_cast<Other>(height) };
	}

	template <IsNumber Other>
		requires(!std::is_same_v<T, Other>)
	[[nodiscard]] auto cast() const
	{
		return std::pair { static_cast<Other>(width), static_cast<Other>(height) };
	}
};

struct Extent : public IExtent<std::uint32_t> { };
struct FloatExtent : public IExtent<float> { };

enum class ImageFormat : std::uint8_t { SRGB, RGB, SBGR, BGR, SRGB32, RGB32, Depth, DepthStencil, Uint, Red };

} // namespace Disarray
