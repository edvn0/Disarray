#include "DisarrayPCH.hpp"

#include "vulkan/Texture.hpp"

#include "core/Log.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Image.hpp"

namespace Disarray::Vulkan {

	Texture::Texture(Disarray::Device& dev, const Disarray::TextureProperties& properties)
		: device(dev)
		, props(properties)
	{
		load_pixels();
		image = make_scope<Vulkan::Image>(device,
			ImageProperties {
				.extent = props.extent,
				.format = props.format,
				.data = pixels,
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
