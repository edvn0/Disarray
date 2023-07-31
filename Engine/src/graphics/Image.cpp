#include "graphics/Image.hpp"

#include "vulkan/Image.hpp"

namespace Disarray {

	Ref<Image> Image::construct(Disarray::Device& device, Disarray::Swapchain& swapchain, const Disarray::ImageProperties& image_properties)
	{
		return make_ref<Vulkan::Image>(device, swapchain, image_properties);
	}

}