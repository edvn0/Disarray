#pragma once

#include "Forward.hpp"
#include "core/Concepts.hpp"
#include "core/DataBuffer.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/UniquelyIdentifiable.hpp"
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

class Image : public ReferenceCountable {
	DISARRAY_OBJECT(Image)
public:
	virtual PixelReadData read_pixel(const glm::vec2&) const = 0;

	virtual Identifier hash() const = 0;

	static Ref<Image> construct(const Disarray::Device&, const ImageProperties&);
};

} // namespace Disarray
