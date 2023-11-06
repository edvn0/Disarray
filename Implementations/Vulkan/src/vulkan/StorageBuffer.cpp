#include "DisarrayPCH.hpp"

#include "vulkan/StorageBuffer.hpp"

namespace Disarray::Vulkan {

StorageBuffer::StorageBuffer(const Disarray::Device& dev, BufferProperties properties)
	: Disarray::StorageBuffer(properties)
	, base_buffer(dev, BufferType::Storage, properties)
{
	info.buffer = supply();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;
}

auto StorageBuffer::size() const -> std::size_t { return base_buffer.size(); }
void StorageBuffer::set_data(const void* data, std::size_t size, std::size_t offset) { return base_buffer.set_data(data, size, offset); }
void StorageBuffer::set_data(const void* data, std::size_t size) { return base_buffer.set_data(data, size, 0); }
auto StorageBuffer::count() const -> std::size_t { return base_buffer.count(); }
auto StorageBuffer::get_raw() -> void* { return base_buffer.get_raw(); }
auto StorageBuffer::get_raw() const -> void* { return base_buffer.get_raw(); }
auto StorageBuffer::supply() const -> VkBuffer { return base_buffer.supply(); }

} // namespace Disarray::Vulkan
