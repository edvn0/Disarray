#include "vulkan/Allocator.hpp"

#include "core/AllocatorConfigurator.hpp"
#include "core/Log.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray {
	void initialise_allocator(Ref<Disarray::Device> device, Ref<Disarray::PhysicalDevice> physical_device, Ref<Disarray::Instance> instance)
	{
		Vulkan::Allocator::initialise(
			cast_to<Vulkan::Device>(device), cast_to<Vulkan::PhysicalDevice>(physical_device), cast_to<Vulkan::Instance>(instance));
	}

	void destroy_allocator() { Vulkan::Allocator::shutdown(); }
} // namespace Disarray

namespace Disarray::Vulkan {

	void Allocator::initialise(Ref<Vulkan::Device> device, Ref<Vulkan::PhysicalDevice> physical_device, Ref<Vulkan::Instance> instance)
	{
		VmaAllocatorCreateInfo create_info {};
		create_info.device = device->supply();
		create_info.physicalDevice = physical_device->supply();
		create_info.instance = instance->supply();

		vmaCreateAllocator(&create_info, &allocator);
	}

	void Allocator::shutdown() { vmaDestroyAllocator(allocator); }

	Allocator::Allocator(const std::string& resource)
		: resource_name(resource)
	{
		Log::debug("Created allocator with name {" + resource_name + "}");
	}

	Allocator::~Allocator() { Log::debug("Destroyed allocator with name {" + resource_name + "}"); }

	VmaAllocation Allocator::allocate_buffer(VkBuffer& buffer, VkBufferCreateInfo buffer_info, const AllocationProperties& props) {
		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = static_cast<VmaMemoryUsage>(props.usage);
		alloc_info.flags = static_cast<VmaAllocationCreateFlags>(props.creation);

		VmaAllocation allocation;
		verify(vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation, nullptr));
		vmaSetAllocationName(allocator, allocation, resource_name.data());

		return allocation;
	}

	void Allocator::deallocate(VmaAllocation allocation, VkBuffer& buffer)
	{
		vmaDestroyBuffer(allocator, buffer, allocation);
	}
} // namespace Disarray::Vulkan