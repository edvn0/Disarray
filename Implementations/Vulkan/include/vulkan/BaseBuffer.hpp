#pragma once

#include <vk_mem_alloc.h>

#include <cstddef>

#include "core/DisarrayObject.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/Device.hpp"
#include "graphics/RendererProperties.hpp"

namespace Disarray::Vulkan {

class VertexBuffer;
class IndexBuffer;
class StorageBuffer;
class UniformBuffer;

class BaseBuffer {
public:
	~BaseBuffer();

protected:
	BaseBuffer(const Disarray::Device&, BufferType type, Disarray::BufferProperties);

	[[nodiscard]] auto size() const -> std::size_t;
	[[nodiscard]] auto count() const -> std::size_t;
	[[nodiscard]] auto get_binding() const -> DescriptorBinding;

	auto get_raw() -> void*;
	[[nodiscard]] auto get_raw() const -> void*;

	void set_data(const void*, std::uint32_t, std::size_t offset = 0ULL);
	void set_data(const void*, std::size_t, std::size_t offset = 0ULL);
	void destroy_buffer();

	[[nodiscard]] auto get_properties() const -> const auto& { return props; }
	[[nodiscard]] auto supply() const -> VkBuffer { return buffer; }

private:
	void create_with_valid_data();
	void create_with_empty_data();

	const Disarray::Device& device;

	BufferType type;
	BufferProperties props;

	VmaAllocationInfo vma_allocation_info {};
	VkBuffer buffer {};
	VmaAllocation allocation {};

	friend class VertexBuffer;
	friend class UniformBuffer;
	friend class IndexBuffer;
	friend class StorageBuffer;
};

} // namespace Disarray::Vulkan
