#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

class UniformBuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(UniformBuffer, BufferProperties)
public:
	virtual auto size() const -> std::size_t = 0;
	virtual void set_data(const void*, std::uint32_t) = 0;

	template <class T>
		requires(sizeof(T) > 0)
	void set_data(T* data)
	{
		set_data(Disarray::bit_cast<const void*>(data), sizeof(T));
	};
};

} // namespace Disarray
