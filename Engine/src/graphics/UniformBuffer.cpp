#include "DisarrayPCH.hpp"

#include "graphics/UniformBuffer.hpp"

#include "vulkan/UniformBuffer.hpp"

namespace Disarray {

auto UniformBuffer::construct(const Disarray::Device& device, Disarray::BufferProperties properties) -> Ref<Disarray::UniformBuffer>
{
	return make_ref<Vulkan::UniformBuffer>(device, properties);
}

auto UniformBuffer::construct_scoped(const Disarray::Device& device, Disarray::BufferProperties properties) -> Scope<Disarray::UniformBuffer>
{
	return make_scope<Vulkan::UniformBuffer>(device, properties);
}

} // namespace Disarray
