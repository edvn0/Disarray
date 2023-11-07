#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/VertexBuffer.hpp"

namespace Disarray::Vulkan {

VertexBuffer::VertexBuffer(const Disarray::Device& dev, Disarray::BufferProperties properties)
	: Disarray::VertexBuffer(properties)
	, base_buffer(dev, BufferType::Vertex, properties)
{
}

auto VertexBuffer::size() const -> std::size_t { return base_buffer.size(); }
void VertexBuffer::set_data(const void* data, std::size_t size, std::size_t offset) { return base_buffer.set_data(data, size, offset); }
void VertexBuffer::set_data(const void* data, std::size_t size) { return base_buffer.set_data(data, size, 0); }
auto VertexBuffer::count() const -> std::size_t { return base_buffer.count(); }
auto VertexBuffer::get_raw() -> void* { return base_buffer.get_raw(); }
auto VertexBuffer::get_raw() const -> void* { return base_buffer.get_raw(); }
auto VertexBuffer::supply() const -> VkBuffer { return base_buffer.supply(); }

} // namespace Disarray::Vulkan
