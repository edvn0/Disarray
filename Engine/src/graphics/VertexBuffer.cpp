#include "DisarrayPCH.hpp"

#include "graphics/VertexBuffer.hpp"

#include "vulkan/VertexBuffer.hpp"

namespace Disarray {

	Ref<VertexBuffer> VertexBuffer::construct(const Disarray::Device& device, const Disarray::BufferProperties& props)
	{
		return make_ref<Vulkan::VertexBuffer>(device, props);
	}

} // namespace Disarray
