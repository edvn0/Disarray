#pragma once

#include <glm/detail/qualifier.hpp>
#include <glm/gtx/norm.hpp>

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <unordered_set>

#include "core/Concepts.hpp"
#include "core/Types.hpp"
#include "graphics/Extent.hpp"

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

enum class ImageFormat : std::uint8_t {
	SRGB,
	RGB,
	SBGR,
	BGR,
	SRGB32,
	RGB32,
	Depth,
	DepthStencil,
	Uint,
	Red,
};

auto to_component_count(ImageFormat format) -> std::uint32_t;
auto to_size(ImageFormat format) -> std::size_t;

auto is_image(const std::filesystem::path& path_to_file) -> bool;

} // namespace Disarray
