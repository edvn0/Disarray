#include "graphics/Texture.hpp"

#include "vulkan/Texture.hpp"

namespace Disarray {

	Ref<Texture> Texture::construct(Disarray::Device& device, Disarray::Swapchain& swapchain,
		const Disarray::TextureProperties& props)
	{
		return make_ref<Vulkan::Texture>(device, swapchain, props);
	}

} // namespace Disarray