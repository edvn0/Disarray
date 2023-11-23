#pragma once

#include "Allocator.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/BaseBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class VertexBuffer : public Disarray::VertexBuffer {
public:
	VertexBuffer(const Disarray::Device&, BufferProperties);
	~VertexBuffer() override = default;

	auto size() const -> std::size_t override;
	void set_data(const void*, std::size_t size, std::size_t offset) override;
	void set_data(const void*, std::size_t size) override;

	auto supply() const -> VkBuffer;
	auto supply() -> VkBuffer;
	auto count() const -> std::size_t override;
	auto get_raw() -> void* override;
	auto get_raw() const -> void* override;

private:
	Vulkan::BaseBuffer base_buffer;
};

} // namespace Disarray::Vulkan
