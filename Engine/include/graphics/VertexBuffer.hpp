#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class PhysicalDevice;

	struct VertexBufferProperties {
		const void* data;
		std::size_t size;
		std::size_t count;
	};

	class VertexBuffer {
	public:
		virtual ~VertexBuffer() = default;

		virtual std::size_t size() = 0;
		virtual void set_data(const void*, std::uint32_t) = 0;

		static Ref<VertexBuffer> construct(Disarray::Device&, Disarray::Swapchain&, const VertexBufferProperties&);
	};

}