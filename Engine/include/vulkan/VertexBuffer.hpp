#pragma once

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

		VkBuffer supply() const override { return buffer; }

	private:
		Ref<Disarray::Device> device;
		VertexBufferProperties props;

		std::size_t vertex_count { 0 };
		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan