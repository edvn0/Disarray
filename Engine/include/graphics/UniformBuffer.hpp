#pragma once

#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class PhysicalDevice;

	class UniformBuffer : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(UniformBuffer)
	public:
		static Ref<UniformBuffer> construct(Disarray::Device&, Disarray::Swapchain&, const Disarray::BufferProperties&);
		virtual std::size_t size() = 0;
		virtual void set_data(const void*, std::uint32_t) = 0;

		template <class T>
			requires(sizeof(T) > 0)
		void set_data(const void* data)
		{
			set_data(data, sizeof(T));
		};
	};

} // namespace Disarray
