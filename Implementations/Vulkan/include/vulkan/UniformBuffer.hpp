#pragma once

#include "graphics/UniformBuffer.hpp"
#include "vulkan/BaseBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class UniformBuffer : public Disarray::UniformBuffer, public Vulkan::BaseBuffer {
	MAKE_SUB_BUFFER(UniformBuffer)
public:
	UniformBuffer(const Disarray::Device&, BufferProperties);

	auto get_buffer_info() const -> const auto& { return buffer_info; }

private:
	void create_buffer_info();

	VkDescriptorBufferInfo buffer_info {};
};

} // namespace Disarray::Vulkan
