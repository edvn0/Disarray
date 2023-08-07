#include "DisarrayPCH.hpp"

#include "graphics/Image.hpp"

#include "vulkan/Image.hpp"

namespace Disarray {

	Ref<Image> Image::construct(Disarray::Device& device, const Disarray::ImageProperties& image_properties)
	{
		return make_ref<Vulkan::Image>(device, image_properties);
	}

} // namespace Disarray
