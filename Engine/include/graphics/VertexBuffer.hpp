#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

class VertexBuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(VertexBuffer, BufferProperties)
public:
	virtual auto size() const -> std::size_t = 0;
	virtual void set_data(const void*, std::uint32_t) = 0;
	virtual void set_data(const void* data, std::size_t size) { return set_data(data, static_cast<std::uint32_t>(size)); };
};

} // namespace Disarray
