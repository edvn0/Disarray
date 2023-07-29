#include "graphics/Texture.hpp"

#include "vulkan/Texture.hpp"

namespace Disarray {

	Ref<Texture> Texture::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,
		const Disarray::TextureProperties& props)
	{
		return make_ref<Vulkan::Texture>(device, swapchain, physical_device, props);
	}

} // namespace Disarray