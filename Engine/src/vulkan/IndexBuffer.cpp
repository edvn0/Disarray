#include "vulkan/IndexBuffer.hpp"

#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"

namespace Disarray::Vulkan {

	IndexBuffer::IndexBuffer(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device, const Disarray::IndexBufferProperties& properties)
	:device(dev), props(properties), index_count(properties.size / sizeof(std::uint32_t))
	{
		Allocator allocator { "VertexBuffer" };

		// create staging buffer
		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = props.size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation
			= allocator.allocate_buffer(staging_buffer, buffer_create_info, { Usage::CPU_TO_GPU });

		{
			AllocationMapper<std::byte> mapper { allocator, staging_buffer_allocation, props.data, props.size };
		}

		VkBufferCreateInfo vertex_buffer_create_info = {};
		vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size = props.size;
		vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		allocation = allocator.allocate_buffer(buffer, vertex_buffer_create_info, { Usage::AUTO_PREFER_DEVICE });

		auto&& [immediate, destruction] = construct_immediate<Vulkan::CommandExecutor>(device, swapchain, physical_device->get_queue_family_indexes());

		VkBufferCopy copy_region = {};
		copy_region.size = props.size;
		vkCmdCopyBuffer(immediate->supply(), staging_buffer, buffer, 1, &copy_region);

		destruction(immediate);
		allocator.deallocate_buffer(staging_buffer_allocation, staging_buffer);
	}

	IndexBuffer::~IndexBuffer() {
		Allocator allocator {"IndexBuffer"};
		allocator.deallocate_buffer(allocation, buffer);
	}

}