#include "vulkan/VertexBuffer.hpp"

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/vulkan_core.h"

namespace Disarray::Vulkan {

	VertexBuffer::VertexBuffer(Disarray::Device& dev, Disarray::Swapchain& swapchain, const Disarray::VertexBufferProperties& properties)
		: device(dev)
		, vertex_count(props.count)
		, props(properties)
	{
		if (props.data) {
			create_with_valid_data(swapchain);
		} else {
			create_with_empty_data();
		}
	}

	VertexBuffer::~VertexBuffer()
	{
		Allocator allocator { "VertexBuffer[" + std::to_string(vertex_count) + "]" };
		allocator.deallocate_buffer(allocation, buffer);
	}

	void VertexBuffer::set_data(const void* data, std::uint32_t size)
	{
		std::memcpy(vma_allocation_info.pMappedData, std::bit_cast<std::byte*>(data), size);
	}

	void VertexBuffer::create_with_valid_data(Disarray::Swapchain& swapchain)
	{
		Allocator allocator { "VertexBuffer" };
		// create staging buffer
		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = props.size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(staging_buffer, buffer_create_info, { Usage::CPU_TO_GPU });

		{
			AllocationMapper<std::byte> mapper { allocator, staging_buffer_allocation, props.data, props.size };
		}

		VkBufferCreateInfo vertex_buffer_create_info = {};
		vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size = props.size;
		vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		allocation = allocator.allocate_buffer(buffer, vertex_buffer_create_info, { Usage::AUTO_PREFER_DEVICE });

		auto&& [immediate, destruction] = construct_immediate<Vulkan::CommandExecutor>(device, swapchain);

		VkBufferCopy copy_region = {};
		copy_region.size = props.size;
		vkCmdCopyBuffer(immediate->supply(), staging_buffer, buffer, 1, &copy_region);

		destruction(immediate);
		allocator.deallocate_buffer(staging_buffer_allocation, staging_buffer);
	}

	void VertexBuffer::create_with_empty_data()
	{
		Allocator allocator("VertexBuffer");

		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = props.size;
		buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		allocation = allocator.allocate_buffer(buffer, vma_allocation_info, buffer_create_info, { Usage::CPU_TO_GPU, Creation::MAPPED_BIT });
	}

} // namespace Disarray::Vulkan