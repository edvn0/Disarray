#pragma once

#include "Forward.hpp"
#include "core/DataBuffer.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/ImageProperties.hpp"

#include <glm/glm.hpp>

namespace Disarray {

struct ImageProperties {
	Extent extent;
	ImageFormat format;
	DataBuffer data;
	std::uint32_t mips { static_cast<std::uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1 };
	bool should_present { false };
	SampleCount samples { SampleCount::One };
	std::string debug_name;
};

class Image : public ReferenceCountable {
	DISARRAY_OBJECT(Image)
public:
	virtual glm::vec4 read_pixel(const glm::vec2&) const = 0;
	static Ref<Image> construct(Disarray::Device&, const ImageProperties&);
};

} // namespace Disarray
