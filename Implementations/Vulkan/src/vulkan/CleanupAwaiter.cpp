#include "DisarrayPCH.hpp"

#include "core/CleanupAwaiter.hpp"

#include <vulkan/vulkan.h>

#include "core/Types.hpp"
#include "vulkan/Device.hpp"

namespace Disarray {

void wait_for_cleanup(Disarray::Device& device) { vkDeviceWaitIdle(supply_cast<Disarray::Vulkan::Device>(device)); }

} // namespace Disarray
