#pragma once

#include <concepts>
#include <cstdint>
#include <limits>

#include "core/Concepts.hpp"
#include "core/Types.hpp"
#include "glm/detail/qualifier.hpp"
#include "glm/gtx/norm.hpp"

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

enum class SamplerFilter : std::uint8_t {
	Linear,
	Nearest,
	Cubic,
};

enum class SampleCount : std::uint8_t {
	One = 0x00000001,
	Two = 0x00000002,
	Four = 0x00000004,
	Eight = 0x00000008,
	Sixteen = 0x00000010,
	ThirtyTwo = 0x00000020,
	SixtyFour = 0x00000040,
};

enum class ImageDimension : std::uint8_t {
	Two,
	Three,
};

enum class Tiling : std::uint8_t { Linear, DeviceOptimal };

struct CopyRegion {
	std::uint32_t mip_level;
	std::uint32_t base_array_layer;
	std::uint32_t width;
	std::uint32_t height;
	std::uint32_t buffer_offset;
};

template <IsNumber T> struct IExtent {
	T width {};
	T height {};

	[[nodiscard]] auto get_size() const -> T { return width * height; }
	[[nodiscard]] auto aspect_ratio() const -> float { return static_cast<float>(width) / static_cast<float>(height); }

	[[nodiscard]] auto valid() const -> bool { return width > 0 && height > 0; }

	auto operator==(const IExtent<T>& other) const -> bool
	{
		return compare_operation(width, other.width) && compare_operation(height, other.height);
	}
	auto operator!=(const IExtent<T>& other) const -> bool
	{
		return !compare_operation(width, other.width) || !compare_operation(height, other.height);
	}

	template <IsNumber Other> auto operator==(const IExtent<Other>& other) const -> bool
	{
		return compare_operation(width, other.width) && compare_operation(height, other.height);
	}
	template <IsNumber Other> auto operator!=(const IExtent<Other>& other) const -> bool
	{
		return !compare_operation(width, other.width) || !compare_operation(height, other.height);
	}

	template <IsNumber Other> auto operator*=(const IExtent<Other>& other) -> IExtent<T>& { return operator*=(other.template as<T>()); }
	auto operator*=(const IExtent<T>& other) -> IExtent<T>&
	{
		width *= other.width;
		height *= other.height;
		return *this;
	}

	template <IsNumber Other> auto operator*(const IExtent<Other>& other) -> IExtent<T> { return operator*(other.template as<T>()); }
	auto operator*(const IExtent<T>& other) -> IExtent<T>
	{
		return IExtent<T> {
			.width = width * other.width,
			.height = height * other.height,
		};
	}

	template <IsNumber Other> auto operator*(Other other) -> IExtent<T> { return operator*(static_cast<T>(other)); }
	auto operator*(T other) -> IExtent<T>
	{
		return IExtent<T> {
			.width = width * other,
			.height = height * other,
		};
	}

	template <IsNumber Other> auto operator/=(const IExtent<Other>& other) -> IExtent<T>& { return operator/=(other.template as<T>()); }
	auto operator/=(const IExtent<T>& other) -> IExtent<T>&
	{
		width *= other.width;
		height *= other.height;
		return *this;
	}

	template <IsNumber Other> auto operator/=(Other other) -> IExtent<T>& { return operator/=(other); }
	auto operator/=(T other) -> IExtent<T>&
	{
		width /= other;
		height /= other;
		return *this;
	}

	constexpr auto compare_operation(std::floating_point auto left, std::floating_point auto right) const
	{
		return std::fabs(left - right) < std::numeric_limits<float>::epsilon();
	}

	constexpr auto compare_operation(std::integral auto left, std::floating_point auto right) const
	{
		return std::fabs(static_cast<decltype(right)>(left) - right) < std::numeric_limits<float>::epsilon();
	}

	constexpr auto compare_operation(std::floating_point auto left, std::integral auto right) const
	{
		return std::fabs(left - static_cast<decltype(left)>(right)) < std::numeric_limits<float>::epsilon();
	}

	constexpr auto compare_operation(std::integral auto left, std::integral auto right) const { return left == right; }

	auto to_string() -> std::string { return fmt::format("{}:{}", width, height); }

	[[nodiscard]] auto sum() const -> float { return width + height; }

	template <IsNumber Other>
		requires(!std::is_same_v<T, Other>)
	auto as() const -> IExtent<Other>
	{
		return {
			.width = static_cast<Other>(width),
			.height = static_cast<Other>(height),
		};
	}

	template <typename VecType>
		requires requires(VecType vector) {
			glm::length(vector);
			vector.x;
			vector.y;
		}
	auto as() const -> VecType
	{
		return VecType {
			static_cast<float>(width),
			static_cast<float>(height),
		};
	}

	template <IsNumber Other>
		requires(!std::is_same_v<T, Other>)
	[[nodiscard]] auto cast() const
	{
		return std::pair {
			static_cast<Other>(width),
			static_cast<Other>(height),
		};
	}
};

struct Extent : public IExtent<std::uint32_t> { };
struct FloatExtent : public IExtent<float> { };

enum class ImageFormat : std::uint8_t { SRGB, RGB, SBGR, BGR, SRGB32, RGB32, Depth, DepthStencil, Uint, Red };

template <std::integral Out = std::uint32_t> inline constexpr auto to_component_count(ImageFormat format)
{
	switch (format) {

	case ImageFormat::SRGB:
	case ImageFormat::RGB:
	case ImageFormat::SBGR:
	case ImageFormat::BGR:
	case ImageFormat::SRGB32:
	case ImageFormat::RGB32:
	case ImageFormat::Depth:
	case ImageFormat::DepthStencil:
	case ImageFormat::Uint:
		return static_cast<Out>(4U);
	case ImageFormat::Red:
		return static_cast<Out>(1U);
	default:
		unreachable();
	}
}

} // namespace Disarray
