#include "graphics/PhysicalDevice.hpp"

#include "vulkan/PhysicalDevice.hpp"

namespace Disarray {

	Ref<PhysicalDevice> PhysicalDevice::construct(Disarray::Instance& instance, Disarray::Surface& surface)
	{
		return make_ref<Vulkan::PhysicalDevice>(instance, surface);
	}

} // namespace Disarray
