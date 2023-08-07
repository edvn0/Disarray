#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class PhysicalDevice;

	class VertexBuffer : public ReferenceCountable {
		DISARRAY_OBJECT(VertexBuffer)
	public:
		static Ref<VertexBuffer> construct(Disarray::Device&, const Disarray::BufferProperties&);
		virtual std::size_t size() = 0;
		virtual void set_data(const void*, std::uint32_t) = 0;
	};

} // namespace Disarray
