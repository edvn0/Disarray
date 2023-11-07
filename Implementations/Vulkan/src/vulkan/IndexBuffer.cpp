#include "DisarrayPCH.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray::Vulkan {

IndexBuffer::IndexBuffer(const Disarray::Device& dev, BufferProperties properties)
	: Disarray::IndexBuffer(properties)
	, base_buffer(dev, BufferType::Index, properties)
{
}

auto IndexBuffer::size() const -> std::size_t { return base_buffer.size(); }
void IndexBuffer::set_data(const void* data, std::size_t size, std::size_t offset) { return base_buffer.set_data(data, size, offset); }
void IndexBuffer::set_data(const void* data, std::size_t size) { return base_buffer.set_data(data, size, 0); }
auto IndexBuffer::count() const -> std::size_t { return base_buffer.count(); }
auto IndexBuffer::get_raw() -> void* { return base_buffer.get_raw(); }
auto IndexBuffer::get_raw() const -> void* { return base_buffer.get_raw(); }
auto IndexBuffer::supply() const -> VkBuffer { return base_buffer.supply(); }

} // namespace Disarray::Vulkan
