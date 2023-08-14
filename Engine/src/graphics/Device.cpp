#include "DisarrayPCH.hpp"

#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Device.hpp"

namespace Disarray {

Scope<Device> Device::construct(Window& window) { return make_scope<Vulkan::Device>(window); }

} // namespace Disarray
