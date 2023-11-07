#pragma once

#include "graphics/StorageBuffer.hpp"
#include "vulkan/BaseBuffer.hpp"

namespace Disarray::Vulkan {

class StorageBuffer : public Disarray::StorageBuffer {
public:
	StorageBuffer(const Disarray::Device& dev, BufferProperties properties);
	~StorageBuffer() override = default;
	auto size() const -> std::size_t override;
	void set_data(const void*, std::size_t size, std::size_t offset) override;
	void set_data(const void*, std::size_t size) override;

	auto supply() const -> VkBuffer;
	auto count() const -> std::size_t override;
	auto get_raw() -> void* override;
	auto get_raw() const -> void* override;

	auto get_descriptor_info() const -> const auto& { return info; }
	static auto get_descriptor_type() { return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; }

private:
	Vulkan::BaseBuffer base_buffer;

	VkDescriptorBufferInfo info {};
};

} // namespace Disarray::Vulkan
