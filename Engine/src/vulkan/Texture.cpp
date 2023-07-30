#include "vulkan/Texture.hpp"

#include "core/DataBuffer.hpp"
#include "core/Log.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageLoader.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/Image.hpp"

namespace Disarray::Vulkan {

	Texture::Texture(
		Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> sc, Ref<Disarray::PhysicalDevice> pd, const Disarray::TextureProperties& properties)
		: device(dev)
		, swapchain(sc)
		, physical_device(pd)
		, props(properties)
	{
		load_pixels();
		image = make_ref<Vulkan::Image>(device, swapchain, physical_device,
			ImageProperties {
				.extent = props.extent,
				.format = props.format,
				.data = pixels,
			});
	}

	Texture::~Texture()
	{
		Log::debug("Destroyed texture.");
	}

	void Texture::recreate(bool should_clean)
	{
		Allocator allocator { "Texture" };
		image->recreate(should_clean);
	}

	void Texture::load_pixels() {
		if (!props.path.empty()){
			ImageLoader loader { props.path, pixels };
		}
	}

} // namespace Disarray::Vulkan