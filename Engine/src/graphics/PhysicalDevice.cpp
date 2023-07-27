#include "graphics/PhysicalDevice.hpp"

#include "vulkan/PhysicalDevice.hpp"

namespace Disarray {

	Scope<PhysicalDevice> PhysicalDevice::construct(Ref<Instance> instance, Ref<Surface> surface)
	{
		return make_scope<Vulkan::PhysicalDevice>(instance, surface);
	}

} // namespace Disarray
