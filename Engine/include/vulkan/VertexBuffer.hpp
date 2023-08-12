#pragma once

#include "Allocator.hpp"
#include "BaseBuffer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class VertexBuffer : public Disarray::VertexBuffer, public Vulkan::BaseBuffer {
		MAKE_SUB_BUFFER(VertexBuffer)
	public:
		VertexBuffer(const Disarray::Device&, const BufferProperties&);
	};

} // namespace Disarray::Vulkan
