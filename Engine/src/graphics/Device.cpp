#include "graphics/Device.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Device.hpp"

namespace Disarray {

	Scope<Device> Device::construct(Ref<PhysicalDevice> physical_device)
	{
		return make_scope<Vulkan::Device>(physical_device);
	}

} // namespace Disarray
