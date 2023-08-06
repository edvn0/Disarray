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
	std::size_t size() override { return BaseBuffer::size(); }                                                                                       \
	~x() override { BaseBuffer::destroy_buffer(); }

	class BaseBuffer : public PropertySupplier<VkBuffer> {
	protected:
		BaseBuffer(Disarray::Device&, Disarray::Swapchain&, BufferType type, const Disarray::BufferProperties&);

		virtual std::size_t size() { return count; }
		virtual void set_data(const void*, std::uint32_t);
		void destroy_buffer();

		const auto& get_properties() const { return props; }

		VkBuffer supply() const override { return buffer; }

	private:
		void create_with_valid_data(Disarray::Swapchain&);
		void create_with_empty_data();
		VkBufferUsageFlags to_vulkan_usage(BufferType type);

		Disarray::Device& device;

		std::size_t count { 0 };
		BufferProperties props;
		BufferType type;

		VmaAllocationInfo vma_allocation_info {};
		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan