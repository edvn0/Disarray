#pragma once

#include "graphics/IndexBuffer.hpp"
#include "vulkan/PropertySupplier.hpp"

#include "vulkan/MemoryAllocator.hpp"

namespace Disarray::Vulkan {

	class IndexBuffer : public Disarray::IndexBuffer, public PropertySupplier<VkBuffer> {
	public:
		IndexBuffer(Disarray::Device& dev, Disarray::Swapchain& swapchain,  const IndexBufferProperties&);
		~IndexBuffer() override;

		std::size_t size() override { return index_count; }
		void set_data(const void *, std::size_t) override;

		VkBuffer supply() const override { return buffer; }

	private:
		void create_with_valid_data(Disarray::Swapchain&);
		void create_with_empty_data();

		Disarray::Device& device;
		IndexBufferProperties props;

		VmaAllocationInfo vma_allocation_info {};

		std::size_t index_count { 0 };
		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan