#pragma once

#include <vulkan/vulkan_core.h>

#include "BaseBuffer.hpp"
#include "graphics/StorageBuffer.hpp"

namespace Disarray::Vulkan {

class StorageBuffer : public Disarray::StorageBuffer, public Vulkan::BaseBuffer {
	MAKE_SUB_BUFFER(StorageBuffer)
public:
	StorageBuffer(const Disarray::Device&, BufferProperties);

	auto get_descriptor_info() const -> const auto& { return info; }

	static auto get_descriptor_type() { return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; }

private:
	VkDescriptorBufferInfo info {};
};

} // namespace Disarray::Vulkan
