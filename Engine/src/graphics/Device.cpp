#include "graphics/Device.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/Device.hpp"

namespace Disarray {

	Scope<Device> Device::construct(Ref<PhysicalDevice> physical_device, Ref<Surface> surface)
	{
		return make_scope<Vulkan::Device>(physical_device, surface);
	}

} // namespace Disarray
