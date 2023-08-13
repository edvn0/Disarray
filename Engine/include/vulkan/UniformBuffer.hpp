#pragma once

#include "Allocator.hpp"
#include "BaseBuffer.hpp"
#include "graphics/UniformBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class UniformBuffer : public Disarray::UniformBuffer, public Vulkan::BaseBuffer {
	MAKE_SUB_BUFFER(UniformBuffer)
public:
	UniformBuffer(Disarray::Device&, const BufferProperties&);
};

} // namespace Disarray::Vulkan
