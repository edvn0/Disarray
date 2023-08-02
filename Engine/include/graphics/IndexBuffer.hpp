#pragma once

#include "core/ReferenceCounted.hpp"
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

	class IndexBuffer : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(IndexBuffer)
	public:
		virtual std::size_t size() = 0;
		virtual void set_data(const void*, std::size_t) = 0;

		static Ref<IndexBuffer> construct(Disarray::Device&, Disarray::Swapchain&, const IndexBufferProperties&);
	};

} // namespace Disarray
