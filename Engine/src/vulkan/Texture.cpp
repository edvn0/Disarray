#include "DisarrayPCH.hpp"

#include "core/Log.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/Texture.hpp"

namespace Disarray::Vulkan {

Texture::Texture(const Disarray::Device& dev, const Disarray::TextureProperties& properties)
	: device(dev)
	, props(properties)
{
	load_pixels();

	if (!props.mips) {
		props.mips = static_cast<std::uint32_t>(std::floor(std::log2(std::max(props.extent.width, props.extent.height)))) + 1;
	}
	image = make_scope<Vulkan::Image>(device,
		ImageProperties {
			.extent = props.extent,
			.format = props.format,
			.data = pixels,
			.mips = *props.mips,
			.debug_name = props.debug_name,
		});
}

Texture::~Texture() { Log::debug("Texture-Destructor", "Destroyed texture {}", props.debug_name); }

void Texture::recreate_texture(bool should_clean) { image->recreate(should_clean, props.extent); }

void Texture::load_pixels()
{
	if (!props.path.empty()) {
		ImageLoader loader { props.path, pixels };
		props.extent = loader.get_extent();
	}
}

} // namespace Disarray::Vulkan
