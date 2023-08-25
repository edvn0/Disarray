#include "DisarrayPCH.hpp"

#include "core/DebugConfigurator.hpp"

#include "core/Types.hpp"
#include "vulkan/DebugMarker.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/PhysicalDevice.hpp"

namespace Disarray::Vulkan {

void vk_initialise_debug_applications(Disarray::Device& device, Disarray::PhysicalDevice& physical_device)
{
	DebugMarker::setup(cast_to<Vulkan::Device>(device).supply(), cast_to<Vulkan::PhysicalDevice>(physical_device).supply());
}

void vk_destroy_debug_applications() { }

} // namespace Disarray::Vulkan

namespace Disarray {
void initialise_debug_applications(Disarray::Device& dev) { Vulkan::vk_initialise_debug_applications(dev, dev.get_physical_device()); }

void destroy_debug_applications() { Vulkan::vk_destroy_debug_applications(); }
} // namespace Disarray
