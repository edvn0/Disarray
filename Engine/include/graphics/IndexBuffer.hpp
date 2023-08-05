#pragma once

#include "BufferProperties.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class PhysicalDevice;

	class IndexBuffer : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(IndexBuffer)
	public:
		static Ref<IndexBuffer> construct(Disarray::Device&, Disarray::Swapchain&, const Disarray::BufferProperties&);
		virtual std::size_t size() = 0;
		virtual void set_data(const void*, std::uint32_t) = 0;
	};

} // namespace Disarray
