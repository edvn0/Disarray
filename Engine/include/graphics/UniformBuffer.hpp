#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

class Device;
class Swapchain;
class PhysicalDevice;

class UniformBuffer : public ReferenceCountable {
	DISARRAY_OBJECT(UniformBuffer)
public:
	static Ref<UniformBuffer> construct(Disarray::Device&, const Disarray::BufferProperties&);
	virtual std::size_t size() const = 0;
	virtual void set_data(const void*, std::uint32_t) = 0;

	template <class T>
		requires(sizeof(T) > 0)
	void set_data(T* data)
	{
		set_data(Disarray::bit_cast<const void*>(data), sizeof(T));
	};
};

} // namespace Disarray
