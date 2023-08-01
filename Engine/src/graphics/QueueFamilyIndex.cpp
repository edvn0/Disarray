#include "graphics/QueueFamilyIndex.hpp"

#include "vulkan/QueueFamilyIndex.hpp"

namespace Disarray {

	Ref<QueueFamilyIndex> QueueFamilyIndex::construct(Disarray::PhysicalDevice& device, Disarray::Surface& surface)
	{
		return make_ref<Vulkan::QueueFamilyIndex>(device, surface);
	}

} // namespace Disarray