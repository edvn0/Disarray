#include "DisarrayPCH.hpp"

#include "graphics/ImageProperties.hpp"

namespace Disarray {

static std::unordered_set<std::string_view> extensions = { ".png", ".jpg", ".jpeg", ".tiff", ".bmp" };

auto is_image(const std::filesystem::path& path_to_file) -> bool
{
	if (path_to_file.has_extension()) {
		std::string extension = path_to_file.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		const auto contains = extensions.contains(extension);
		return contains;
	}
	return false;
}

auto to_component_count(ImageFormat format) -> std::uint32_t
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
		return 4U;
	case ImageFormat::Red:
		return 1U;
	default:
		unreachable();
	}
}

auto to_size(ImageFormat format) -> std::size_t
{
	switch (format) {
	case ImageFormat::SRGB:
	case ImageFormat::RGB:
	case ImageFormat::SBGR:
	case ImageFormat::BGR:
	case ImageFormat::Depth:
	case ImageFormat::DepthStencil:
	case ImageFormat::Uint:
	case ImageFormat::Red:
		return sizeof(float);
	case ImageFormat::RGB32:
	case ImageFormat::SRGB32:
		return 4 * sizeof(float);
	default:
		unreachable();
	}
}

} // namespace Disarray
