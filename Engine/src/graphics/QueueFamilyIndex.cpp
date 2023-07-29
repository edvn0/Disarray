#include "graphics/QueueFamilyIndex.hpp"

#include "vulkan/QueueFamilyIndex.hpp"

namespace Disarray {

	Ref<QueueFamilyIndex> QueueFamilyIndex::construct(Ref<Disarray::PhysicalDevice> device, Ref<Disarray::Surface> surface)
	{
		return make_ref<Vulkan::QueueFamilyIndex>(device, surface);
	}

}