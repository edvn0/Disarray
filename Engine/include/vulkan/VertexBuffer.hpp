#pragma once

#include "Allocator.hpp"
#include "graphics/VertexBuffer.hpp"
#include "vulkan/MemoryAllocator.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class VertexBuffer : public Disarray::VertexBuffer, public PropertySupplier<VkBuffer> {
	public:
		VertexBuffer(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,
			const VertexBufferProperties&);
		~VertexBuffer() override;

		std::size_t size() override { return vertex_count; }
		void set_data(const void*, std::uint32_t) override;

		VkBuffer supply() const override { return buffer; }

	private:
		void create_with_valid_data(Ref<Disarray::Swapchain>, Ref<Disarray::PhysicalDevice>);
		void create_with_empty_data();

		Ref<Disarray::Device> device;
		VertexBufferProperties props;

		VmaAllocationInfo vma_allocation_info {};

		std::size_t vertex_count { 0 };
		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan