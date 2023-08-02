#pragma once

#include "Allocator.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class VertexBuffer : public Disarray::VertexBuffer, public PropertySupplier<VkBuffer> {
	public:
		VertexBuffer(Disarray::Device&, Disarray::Swapchain&, const VertexBufferProperties&);
		~VertexBuffer() override;

		std::size_t size() override { return vertex_count; }
		void set_data(const void*, std::uint32_t) override;

		VkBuffer supply() const override { return buffer; }

	private:
		void create_with_valid_data(Disarray::Swapchain&);
		void create_with_empty_data();

		Disarray::Device& device;
		VertexBufferProperties props;
		std::size_t vertex_count { 0 };

		VmaAllocationInfo vma_allocation_info {};

		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan
