#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class PhysicalDevice;

	struct IndexBufferProperties {
		const void* data;
		std::size_t size;
		std::size_t count;
	};

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() = default;

		virtual std::size_t size() = 0;

		static Ref<IndexBuffer> construct(Ref<Disarray::Device> dev, Ref<Disarray::Swapchain> swapchain, Ref<Disarray::PhysicalDevice> physical_device,  const IndexBufferProperties&);
	};

}