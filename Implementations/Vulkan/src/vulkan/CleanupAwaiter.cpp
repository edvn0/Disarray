#include "DisarrayPCH.hpp"

#include "core/CleanupAwaiter.hpp"

#include <vulkan/vulkan.h>

#include "core/Types.hpp"
#include "vulkan/Device.hpp"

namespace Disarray {

void wait_for_idle(Disarray::Device& device) { vkDeviceWaitIdle(supply_cast<Disarray::Vulkan::Device>(device)); }
void wait_for_idle(const Disarray::Device& device) { vkDeviceWaitIdle(supply_cast<Disarray::Vulkan::Device>(device)); }

} // namespace Disarray
