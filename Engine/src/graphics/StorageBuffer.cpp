#include "DisarrayPCH.hpp"

#include "graphics/StorageBuffer.hpp"

#include "vulkan/StorageBuffer.hpp"

namespace Disarray {

auto StorageBuffer::construct(const Disarray::Device& device, Disarray::BufferProperties properties) -> Ref<Disarray::StorageBuffer>
{
	return make_ref<Vulkan::StorageBuffer>(device, properties);
}

auto StorageBuffer::construct_scoped(const Disarray::Device& device, Disarray::BufferProperties properties) -> Scope<Disarray::StorageBuffer>
{
	return make_scope<Vulkan::StorageBuffer>(device, properties);
}

} // namespace Disarray
