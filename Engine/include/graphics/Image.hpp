#pragma once

#include "Forward.hpp"
#include "core/Concepts.hpp"
#include "core/DataBuffer.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/ImageProperties.hpp"

#include <glm/glm.hpp>
#include <variant>

namespace Disarray {

struct ImageProperties {
	Extent extent;
	ImageFormat format;
	DataBuffer data;
	std::uint32_t mips { static_cast<std::uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1 };
	bool should_present { false };
	SampleCount samples { SampleCount::One };
	Tiling tiling { Tiling::DeviceOptimal };
	std::string debug_name;
};

// Colour, identity, emtpy
using PixelReadData = std::variant<glm::vec4, std::uint32_t, std::monostate>;

template <class T> constexpr auto read_from_pixel_data(const PixelReadData& data)
{
	ensure(holds_alternative<T>(data));
	return std::get<T>(data);
}

class Image : public ReferenceCountable {
	DISARRAY_OBJECT(Image)
public:
	virtual PixelReadData read_pixel(const glm::vec2&) const = 0;

	template <class T>
		requires AnyOf<T, glm::vec4, std::uint32_t>
	auto get_pixel_data(const glm::vec2& coord) const
	{
		return read_from_pixel_data<T>(read_pixel(coord));
	}

	static Ref<Image> construct(const Disarray::Device&, const ImageProperties&);
};

} // namespace Disarray
