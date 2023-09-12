#include "DisarrayPCH.hpp"

#include "vulkan/Texture.hpp"

#include <utility>

#include "core/Log.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Image.hpp"

namespace Disarray::Vulkan {

Texture::Texture(const Disarray::Device& dev, Disarray::TextureProperties properties)
	: Disarray::Texture(std::move(properties))
	, device(dev)
{
	load_pixels();

	if (!props.mips) {
		props.mips = static_cast<std::uint32_t>(std::floor(std::log2(std::max(props.extent.width, props.extent.height)))) + 1;
	}
	image = make_scope<Vulkan::Image>(device,
		ImageProperties {
			.extent = props.extent,
			.format = props.format,
			.data = DataBuffer { pixels },
			.mips = *props.mips,
			.locked_extent = props.locked_extent,
			.debug_name = props.debug_name,
		});
}

Texture::~Texture() { }

void Texture::recreate_texture(bool should_clean) { image->recreate(should_clean, props.extent); }

void Texture::load_pixels()
{
	if (!props.path.empty()) {
		ImageLoader loader { props.path, pixels };
		props.extent = loader.get_extent();
	}
}

} // namespace Disarray::Vulkan
