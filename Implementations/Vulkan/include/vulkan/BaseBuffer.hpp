#pragma once

#include <vk_mem_alloc.h>

#include "core/DisarrayObject.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/Device.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

#define MAKE_SUB_BUFFER(x)                                                                                                                           \
	DISARRAY_MAKE_NONCOPYABLE(x)                                                                                                                     \
                                                                                                                                                     \
public:                                                                                                                                              \
	auto supply() const -> VkBuffer override { return BaseBuffer::supply(); }                                                                        \
	void set_data(const void* data, std::uint32_t size) override { BaseBuffer::set_data(data, size); }                                               \
	auto size() const -> std::size_t override { return BaseBuffer::size(); }                                                                         \
	~x() override { BaseBuffer::destroy_buffer(); }

class BaseBuffer : public PropertySupplier<VkBuffer> {
public:
	~BaseBuffer() override = default;

protected:
	BaseBuffer(const Disarray::Device&, BufferType type, Disarray::BufferProperties);

	[[nodiscard]] virtual auto size() const -> std::size_t;

	virtual void set_data(const void*, std::uint32_t);
	void destroy_buffer();

	[[nodiscard]] auto get_properties() const -> const auto& { return props; }

	[[nodiscard]] auto supply() const -> VkBuffer override { return buffer; }

private:
	void create_with_valid_data();
	void create_with_empty_data();
	auto to_vulkan_usage(BufferType type) -> VkBufferUsageFlags;

	const Disarray::Device& device;

	BufferType type;
	BufferProperties props;
	std::size_t count { 0 };

	VmaAllocationInfo vma_allocation_info {};
	VkBuffer buffer;
	VmaAllocation allocation;
};

} // namespace Disarray::Vulkan
