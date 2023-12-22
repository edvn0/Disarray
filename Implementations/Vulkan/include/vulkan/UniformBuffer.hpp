#pragma once

#include <vulkan/vulkan.h>

#include "graphics/UniformBuffer.hpp"
#include "vulkan/BaseBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class UniformBuffer : public Disarray::UniformBuffer {
public:
	using base_type = Disarray::UniformBuffer;

	UniformBuffer(const Disarray::Device&, BufferProperties);
	~UniformBuffer() override = default;

	auto size() const -> std::size_t override;
	auto count() const -> std::size_t override;
	auto get_binding() const -> DescriptorBinding override;

	auto supply() const -> VkBuffer;
	auto get_raw() -> void* override;
	auto get_raw() const -> void* override;

	auto get_descriptor_info() const -> const auto& { return buffer_info; }
	static auto get_descriptor_type() { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }

	void set_data(const void*, std::size_t size, std::size_t offset) override;
	void set_data(const void*, std::size_t size) override;

private:
	Vulkan::BaseBuffer base_buffer;
	VkDescriptorBufferInfo buffer_info {};
	void create_buffer_info();
};

} // namespace Disarray::Vulkan
