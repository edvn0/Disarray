#include "DisarrayPCH.hpp"

#include "vulkan/Texture.hpp"

#include "core/Log.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Image.hpp"

namespace Disarray::Vulkan {

	Texture::Texture(Disarray::Device& dev, Disarray::Swapchain& sc, const Disarray::TextureProperties& properties)
		: device(dev)
		, props(properties)
	{
		load_pixels();
		image = make_scope<Vulkan::Image>(device, sc,
			ImageProperties {
				.extent = props.extent,
				.format = props.format,
				.data = pixels,
				.should_present = props.should_present,
				.debug_name = props.debug_name,
			});
	}

	Texture::~Texture() { Log::debug("Texture-Destructor", "Destroyed texture " + props.debug_name); }

	void Texture::recreate_texture(bool should_clean) { image->recreate(should_clean); }

	void Texture::load_pixels()
	{
		if (!props.path.empty()) {
			ImageLoader loader { props.path, pixels };
		}
	}

} // namespace Disarray::Vulkan
