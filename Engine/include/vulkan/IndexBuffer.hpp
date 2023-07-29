#pragma once

#include "graphics/IndexBuffer.hpp"
#include "vulkan/PropertySupplier.hpp"

#include <vk_mem_alloc.h>

namespace Disarray::Vulkan {

	class IndexBuffer : public Disarray::IndexBuffer, public PropertySupplier<VkBuffer> {
	public:
		IndexBuffer(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,  const IndexBufferProperties&);
		~IndexBuffer() override;

		std::size_t size() override { return index_count; }

		VkBuffer supply() const override { return buffer; }

	private:
		Ref<Disarray::Device> device;
		IndexBufferProperties props;

		std::size_t index_count { 0 };
		VkBuffer buffer;
		VmaAllocation allocation;
	};

} // namespace Disarray::Vulkan