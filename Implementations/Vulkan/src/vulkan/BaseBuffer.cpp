#include "vulkan/BaseBuffer.hpp"

#include <magic_enum.hpp>

#include "core/Ensure.hpp"
#include "graphics/BufferProperties.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/PhysicalDevice.hpp"

namespace Disarray::Vulkan {

static auto to_vulkan_usage(BufferType buffer_type) -> VkBufferUsageFlags
{
	switch (buffer_type) {
	case BufferType::Vertex:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case BufferType::Index:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case BufferType::Uniform:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case BufferType::Storage:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	default:
		unreachable();
	}
}

auto fix_buffer_validity(const Disarray::Device& device, BufferType type, Disarray::BufferProperties& properties) -> void
{
	const auto& physical = device.get_physical_device();
	const auto& vk_physical = cast_to<Vulkan::PhysicalDevice>(physical);
	const double count_size_percentage = static_cast<double>(properties.count) / static_cast<double>(properties.size);

	if (type == BufferType::Uniform) {
		if (properties.size > vk_physical.get_limits().maxUniformBufferRange) {
			properties.size = vk_physical.get_limits().maxUniformBufferRange;
		}
	} else if (type == BufferType::Storage) {
		if (properties.size > vk_physical.get_limits().maxStorageBufferRange) {
			properties.size = vk_physical.get_limits().maxStorageBufferRange;
		}
	} else {
		return;
	}
	properties.count = static_cast<std::size_t>(count_size_percentage * static_cast<double>(properties.size));
}

BaseBuffer::BaseBuffer(const Disarray::Device& dev, BufferType buffer_type, Disarray::BufferProperties properties)
	: device(dev)
	, type(buffer_type)
	, props(properties)
{
	fix_buffer_validity(dev, type, props);

	if (props.data != nullptr) {
		create_with_valid_data();
	} else {
		create_with_empty_data();
	}
}

BaseBuffer::~BaseBuffer() { destroy_buffer(); }

void BaseBuffer::create_with_valid_data()
{
	Allocator allocator { fmt::format("{}Buffer", magic_enum::enum_name(type)) };
	// create staging buffer
	auto buffer_create_info = vk_structures<VkBufferCreateInfo> {}();
	buffer_create_info.size = props.size;
	buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkBuffer staging_buffer {};
	VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(staging_buffer, buffer_create_info, { Usage::CPU_TO_GPU });

	{
		AllocationMapper<std::byte> mapper { allocator, staging_buffer_allocation, props.data, props.size };
	}

	auto typed_create_info = vk_structures<VkBufferCreateInfo> {}();
	typed_create_info.size = props.size;
	typed_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | to_vulkan_usage(type);
	auto usage = type != BufferType::Uniform ? Usage::AUTO_PREFER_DEVICE : Usage::AUTO_PREFER_HOST;
	// If we don't want it to always be mapped, be explicit here
	if (!props.always_mapped) {
		usage = Usage::AUTO_PREFER_DEVICE;
	}
	auto creation = Creation::HOST_ACCESS_RANDOM_BIT;
	if (const auto is_uniform = type == BufferType::Uniform) {
		creation |= Creation::MAPPED_BIT;
	}

	allocation = allocator.allocate_buffer(buffer, typed_create_info,
		{
			.usage = usage,
			.creation = creation,
		});

	{
		auto executor = construct_immediate(device);
		VkBufferCopy copy_region = {};
		copy_region.size = props.size;
		vkCmdCopyBuffer(executor->supply(), staging_buffer, buffer, 1, &copy_region);
	}
	allocator.deallocate_buffer(staging_buffer_allocation, staging_buffer);
}

void BaseBuffer::create_with_empty_data()
{
	Allocator allocator { fmt::format("{}Buffer", magic_enum::enum_name(type)) };

	auto buffer_create_info = vk_structures<VkBufferCreateInfo> {}();
	buffer_create_info.size = props.size;
	buffer_create_info.usage = to_vulkan_usage(type);

	const auto is_uniform = type == BufferType::Uniform;

	auto usage = is_uniform ? Usage::AUTO_PREFER_HOST : Usage::CPU_TO_GPU;
	auto creation = Creation::MAPPED_BIT;
	if (is_uniform) {
		creation |= Creation::HOST_ACCESS_RANDOM_BIT;
	}

	allocation = allocator.allocate_buffer(buffer, vma_allocation_info, buffer_create_info,
		{
			.usage = usage,
			.creation = creation,
		});
}

void BaseBuffer::set_data(const void* data, std::uint32_t size, std::size_t offset)
{
	if (props.always_mapped) {
		std::memcpy(vma_allocation_info.pMappedData, data, size);
		return;
	}

	Allocator allocator { fmt::format("{}Buffer", magic_enum::enum_name(type)) };
	auto* output = allocator.map_memory<std::byte>(allocation);
	std::memcpy(output, data, size);
	allocator.unmap_memory(allocation);
}

void BaseBuffer::set_data(const void* data, std::size_t size, std::size_t offset) { return set_data(data, static_cast<std::uint32_t>(size), offset); }

void BaseBuffer::destroy_buffer()
{
	Allocator allocator { fmt::format("{}Buffer", magic_enum::enum_name(type)) };
	allocator.deallocate_buffer(allocation, buffer);
}

auto BaseBuffer::size() const -> std::size_t
{
	if (type == BufferType::Uniform) {
		return props.size;
	}

	return props.count;
}

auto BaseBuffer::count() const -> std::size_t { return props.count; }

auto BaseBuffer::get_raw() -> void*
{
	if (props.always_mapped) {
		return vma_allocation_info.pMappedData;
	}

	ensure(false, "Never here");
	return nullptr;
}
auto BaseBuffer::get_raw() const -> void*
{
	if (props.always_mapped) {
		return vma_allocation_info.pMappedData;
	}

	ensure(false, "Never here");
	return nullptr;
}

} // namespace Disarray::Vulkan
