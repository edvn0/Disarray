#include "DisarrayPCH.hpp"

#include "graphics/VertexBuffer.hpp"

#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

auto VertexBuffer::construct(const Disarray::Device& device, Disarray::BufferProperties properties) -> Ref<Disarray::VertexBuffer>
{
	return make_ref<Vulkan::VertexBuffer>(device, properties);
}

auto VertexBuffer::construct_scoped(const Disarray::Device& device, Disarray::BufferProperties properties) -> Scope<Disarray::VertexBuffer>
{
	return make_scope<Vulkan::VertexBuffer>(device, properties);
}

} // namespace Disarray
