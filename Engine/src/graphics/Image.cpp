#include "graphics/Image.hpp"

#include "vulkan/Image.hpp"

namespace Disarray {

	Ref<Image> Image::construct(Ref<Disarray::Device> device, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, const Disarray::ImageProperties& image_properties)
	{
		return make_ref<Vulkan::Image>(device, swapchain, physical_device, image_properties);
	}

}