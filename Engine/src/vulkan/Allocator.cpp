#include "vulkan/Allocator.hpp"

#include "core/AllocatorConfigurator.hpp"
#include "core/Ensure.hpp"
#include "core/Log.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/PhysicalDevice.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray {
	void initialise_allocator(Disarray::Device& dev, Disarray::Instance& inst)
	{
		auto& device = cast_to<Vulkan::Device>(dev);
		auto& physical = device.get_physical_device();
		auto& vk_physical = cast_to<Vulkan::PhysicalDevice>(physical);
		auto& instance = cast_to<Vulkan::Instance>(inst);
		Vulkan::Allocator::initialise(device, vk_physical, instance);
	}

	void destroy_allocator() { Vulkan::Allocator::shutdown(); }
} // namespace Disarray

namespace Disarray::Vulkan {

	void Allocator::initialise(Vulkan::Device& device, Vulkan::PhysicalDevice& physical_device, Vulkan::Instance& instance)
	{
		VmaAllocatorCreateInfo create_info {};
		create_info.device = *device;
		create_info.physicalDevice = *physical_device;
		create_info.instance = *instance;

		vmaCreateAllocator(&create_info, &allocator);
	}

	void Allocator::shutdown() { vmaDestroyAllocator(allocator); }

	Allocator::Allocator(const std::string& resource)
		: resource_name(resource)
	{
	}

	Allocator::~Allocator() { }

	VmaAllocation Allocator::allocate_buffer(VkBuffer& buffer, VkBufferCreateInfo buffer_info, const AllocationProperties& props)
	{
		ensure(allocator != nullptr, "Allocator was null.");
		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = static_cast<VmaMemoryUsage>(props.usage);
		alloc_info.flags = static_cast<VmaAllocationCreateFlags>(props.creation);

		VmaAllocation allocation;
		verify(vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation, nullptr));
		vmaSetAllocationName(allocator, allocation, resource_name.data());

		return allocation;
	}

	VmaAllocation Allocator::allocate_buffer(
		VkBuffer& buffer, VmaAllocationInfo& allocation_info, VkBufferCreateInfo buffer_info, const AllocationProperties& props)
	{
		ensure(allocator != nullptr, "Allocator was null.");
		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = static_cast<VmaMemoryUsage>(props.usage);
		alloc_info.flags = static_cast<VmaAllocationCreateFlags>(props.creation);

		VmaAllocation allocation;
		verify(vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation, &allocation_info));
		vmaSetAllocationName(allocator, allocation, resource_name.data());

		return allocation;
	}

	VmaAllocation Allocator::allocate_image(VkImage& image, VkImageCreateInfo image_create_info, const AllocationProperties& props)
	{
		ensure(allocator != nullptr, "Allocator was null.");
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = static_cast<VmaMemoryUsage>(props.usage);

		VmaAllocation allocation;
		verify(vmaCreateImage(allocator, &image_create_info, &allocation_create_info, &image, &allocation, nullptr));
		vmaSetAllocationName(allocator, allocation, resource_name.data());

		return allocation;
	}

	void Allocator::deallocate_buffer(VmaAllocation allocation, VkBuffer& buffer)
	{
		ensure(allocator != nullptr, "Allocator was null.");
		vmaDestroyBuffer(allocator, buffer, allocation);
	}

	void Allocator::deallocate_image(VmaAllocation allocation, VkImage& image)
	{
		ensure(allocator != nullptr, "Allocator was null.");
		vmaDestroyImage(allocator, image, allocation);
	}

} // namespace Disarray::Vulkan
