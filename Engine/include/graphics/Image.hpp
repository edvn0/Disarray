#pragma once

#include "Forward.hpp"

#include <glm/glm.hpp>

#include <array>
#include <variant>

#include "core/Concepts.hpp"
#include "core/DataBuffer.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

struct ImageProperties {
	Extent extent;
	ImageFormat format;
	DataBuffer data;
	std::uint32_t mips { static_cast<std::uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1 };
	SampleCount samples { SampleCount::One };
	Tiling tiling { Tiling::DeviceOptimal };
	bool locked_extent { false };
	struct SamplerModeUVW {
		SamplerMode u = SamplerMode::Repeat;
		SamplerMode v = SamplerMode::Repeat;
		SamplerMode w = SamplerMode::Repeat;
	} sampler_modes {};
	std::vector<CopyRegion> copy_regions {};
	SamplerFilter filter { SamplerFilter::Linear };
	BorderColour border_colour { BorderColour::FloatOpaqueWhite };
	std::uint32_t layers { 1 };
	ImageDimension dimension { ImageDimension::Two };
	std::string debug_name;
};

// Colour, identity, emtpy
using PixelReadData = std::variant<glm::vec4, std::uint32_t, std::monostate>;

class Image : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Image, ImageProperties)
public:
	virtual auto read_pixel(const glm::vec2&) const -> PixelReadData = 0;
	virtual auto hash() const -> Identifier = 0;

	virtual void construct_using(CommandExecutor&) = 0;

	static void write_to_file(std::string_view path, const Image& image, const void* data);
	static void write_to_file(std::string_view path, const Image& image);
};

} // namespace Disarray
