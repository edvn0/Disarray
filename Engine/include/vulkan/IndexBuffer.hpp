#pragma once

#include "graphics/IndexBuffer.hpp"
#include "vulkan/PropertySupplier.hpp"

#include "vulkan/MemoryAllocator.hpp"

namespace Disarray::Vulkan {

	class IndexBuffer : public Disarray::IndexBuffer, public PropertySupplier<VkBuffer> {
	public:
		IndexBuffer(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,  const IndexBufferProperties&);
		~IndexBuffer() override;

		std::size_t size() override { return index_count; }
		void set_data(const void *, std::size_t) override;

		VkBuffer supply() const override { return buffer; }

	private:
		void create_with_valid_data(Ref<Disarray::Swapchain>, Ref<Disarray::PhysicalDevice>);
		void create_with_empty_data();

		Ref<Disarray::Device> device;
		IndexBufferProperties props;

		VmaAllocationInfo vma_allocation_info {};

		std::size_t index_count { 0 };
		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan