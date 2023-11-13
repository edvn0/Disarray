#include "DisarrayPCH.hpp"

#include <stb_image_write.h>

#include "graphics/Image.hpp"
#include "vulkan/Image.hpp"

namespace Disarray {

auto Image::construct(const Disarray::Device& device, ImageProperties image_properties) -> Ref<Disarray::Image>
{
	return make_ref<Vulkan::Image>(device, std::move(image_properties));
}

void Image::write_to_file(std::string_view path, const Image& image, const void* data)
{
	const auto& props = image.get_properties();
	const auto& [w, h] = props.extent;
	stbi_write_bmp(path.data(), static_cast<int>(w), static_cast<int>(h), 4, data);
}

void Image::write_to_file(std::string_view path, const Image& image)
{
	const auto& props = image.get_properties();
	const auto& [w, h] = props.extent;
	auto comp = to_component_count(image.get_properties().format);
	stbi_write_bmp(path.data(), static_cast<int>(w), static_cast<int>(h), comp, image.get_properties().data.get_data());
}

} // namespace Disarray
