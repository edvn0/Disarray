#pragma once

#include "BufferProperties.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class PhysicalDevice;

	class IndexBuffer : public ReferenceCountable {
		DISARRAY_OBJECT(IndexBuffer)
	public:
		static Ref<IndexBuffer> construct(const Disarray::Device&, const Disarray::BufferProperties&);
		virtual std::size_t size() const = 0;
		virtual void set_data(const void*, std::uint32_t) = 0;
	};

} // namespace Disarray
