#pragma once

#include "BaseBuffer.hpp"
#include "graphics/IndexBuffer.hpp"

namespace Disarray::Vulkan {

class IndexBuffer : public Disarray::IndexBuffer, public Vulkan::BaseBuffer {
	MAKE_SUB_BUFFER(IndexBuffer)
public:
	IndexBuffer(const Disarray::Device&, BufferProperties);
};

} // namespace Disarray::Vulkan
