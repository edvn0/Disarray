#pragma once

#include "BaseBuffer.hpp"
#include "graphics/IndexBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class IndexBuffer : public Disarray::IndexBuffer, public Vulkan::BaseBuffer {
		MAKE_SUB_BUFFER(IndexBuffer)
	public:
		IndexBuffer(Disarray::Device&, const BufferProperties&);
	};

} // namespace Disarray::Vulkan
