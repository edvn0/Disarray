#include "DisarrayPCH.hpp"

#include "graphics/Image.hpp"

#include <fmt/format.h>

#include "vulkan/Image.hpp"

namespace fmt {
auto formatter<Disarray::ImageFormat>::format(Disarray::ImageFormat image_format, format_context& ctx) const
{
	switch (image_format) {
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

auto formatter<Disarray::SampleCount>::format(Disarray::SampleCount samples, format_context& ctx) const
{
	switch (samples) {

	case Disarray::SampleCount::One:
		return formatter<std::string_view>::format(fmt::format("[{}]", 1), ctx);
	case Disarray::SampleCount::Two:
		return formatter<std::string_view>::format(fmt::format("[{}]", 2), ctx);
	case Disarray::SampleCount::Four:
		return formatter<std::string_view>::format(fmt::format("[{}]", 4), ctx);
	case Disarray::SampleCount::Eight:
		return formatter<std::string_view>::format(fmt::format("[{}]", 8), ctx);
	case Disarray::SampleCount::Sixteen:
		return formatter<std::string_view>::format(fmt::format("[{}]", 16), ctx);
	case Disarray::SampleCount::ThirtyTwo:
		return formatter<std::string_view>::format(fmt::format("[{}]", 32), ctx);
	case Disarray::SampleCount::SixtyFour:
		return formatter<std::string_view>::format(fmt::format("[{}]", 64), ctx);
	default:
		Disarray::unreachable();
	}
}

} // namespace fmt

namespace Disarray {

Ref<Image> Image::construct(const Device& device, const ImageProperties& image_properties)
{
	return make_ref<Vulkan::Image>(device, image_properties);
}

} // namespace Disarray
