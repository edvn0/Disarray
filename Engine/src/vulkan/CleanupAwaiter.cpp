#include "core/CleanupAwaiter.hpp"
#include "core/Types.hpp"

#include "vulkan/Device.hpp"

#include <vulkan/vulkan.h>

namespace Disarray {

	void wait_for_cleanup(Ref<Device> device)
	{
		vkDeviceWaitIdle(supply_cast<Disarray::Vulkan::Device>(device));
	}

}