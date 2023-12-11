#include "DisarrayPCH.hpp"

#include <vulkan/vulkan.h>

#include "graphics/PhysicalDevice.hpp"
#include "vulkan/Allocator.hpp"
#include "vulkan/CommandExecutor.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/UniformBuffer.hpp"

namespace Disarray::Vulkan {

UniformBuffer::UniformBuffer(const Disarray::Device& dev, Disarray::BufferProperties properties)
	: Disarray::UniformBuffer(properties)
	, base_buffer(dev, BufferType::Uniform, properties)
{
	create_buffer_info();
}

void UniformBuffer::create_buffer_info()
{
	buffer_info.buffer = supply();
	buffer_info.offset = 0;
	buffer_info.range = size();
}

auto UniformBuffer::size() const -> std::size_t { return base_buffer.size(); }
auto UniformBuffer::get_binding() const -> DescriptorBinding { return base_buffer.get_binding(); }
void UniformBuffer::set_data(const void* data, std::size_t size, std::size_t offset) { return base_buffer.set_data(data, size, offset); }
void UniformBuffer::set_data(const void* data, std::size_t size) { return base_buffer.set_data(data, size, 0); }
auto UniformBuffer::count() const -> std::size_t { return base_buffer.count(); }
auto UniformBuffer::get_raw() -> void* { return base_buffer.get_raw(); }
auto UniformBuffer::get_raw() const -> void* { return base_buffer.get_raw(); }
auto UniformBuffer::supply() const -> VkBuffer { return base_buffer.supply(); }

} // namespace Disarray::Vulkan
