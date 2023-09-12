#include "DisarrayPCH.hpp"

#include "graphics/IndexBuffer.hpp"

#include "vulkan/IndexBuffer.hpp"

namespace Disarray {

auto IndexBuffer::construct(const Disarray::Device& device, Disarray::BufferProperties properties) -> Ref<Disarray::IndexBuffer>
{
	return make_ref<Vulkan::IndexBuffer>(device, properties);
}

auto IndexBuffer::construct_scoped(const Disarray::Device& device, Disarray::BufferProperties properties) -> Scope<Disarray::IndexBuffer>
{
	return make_scope<Vulkan::IndexBuffer>(device, properties);
}

} // namespace Disarray
