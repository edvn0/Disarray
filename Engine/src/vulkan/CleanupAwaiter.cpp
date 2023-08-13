#include "DisarrayPCH.hpp"

#include "core/CleanupAwaiter.hpp"
#include "core/Types.hpp"
#include "vulkan/Device.hpp"

#include <vulkan/vulkan.h>

namespace Disarray {

void wait_for_cleanup(Disarray::Device& device) { vkDeviceWaitIdle(supply_cast<Disarray::Vulkan::Device>(device)); }

} // namespace Disarray
