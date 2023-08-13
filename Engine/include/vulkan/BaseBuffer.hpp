#pragma once

#include "PropertySupplier.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/Device.hpp"
#include "graphics/Swapchain.hpp"

#include <vk_mem_alloc.h>

namespace Disarray::Vulkan {

#define MAKE_SUB_BUFFER(x)                                                                                                                           \
public:                                                                                                                                              \
	VkBuffer supply() const override { return BaseBuffer::supply(); }                                                                                \
	void set_data(const void* data, std::uint32_t size) override { BaseBuffer::set_data(data, size); }                                               \
	std::size_t size() const override { return BaseBuffer::size(); }                                                                                 \
	~x() override { BaseBuffer::destroy_buffer(); }

class BaseBuffer : public PropertySupplier<VkBuffer> {
protected:
	BaseBuffer(const Disarray::Device&, BufferType type, const Disarray::BufferProperties&);
	virtual ~BaseBuffer() override = default;

	virtual std::size_t size() const
	{
		if (type == BufferType::Uniform)
			return props.size;
		else
			return count;
	}

	virtual void set_data(const void*, std::uint32_t);
	void destroy_buffer();

	const auto& get_properties() const { return props; }

	VkBuffer supply() const override { return buffer; }

private:
	void create_with_valid_data();
	void create_with_empty_data();
	VkBufferUsageFlags to_vulkan_usage(BufferType type);

	const Disarray::Device& device;

	BufferType type;
	BufferProperties props;
	std::size_t count { 0 };

	VmaAllocationInfo vma_allocation_info {};
	VkBuffer buffer;
	VmaAllocation allocation;
};

} // namespace Disarray::Vulkan
