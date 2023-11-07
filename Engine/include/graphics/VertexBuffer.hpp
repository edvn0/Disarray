#pragma once

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

class VertexBuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(VertexBuffer, BufferProperties)
public:
	[[nodiscard]] virtual auto size() const -> std::size_t = 0;
	[[nodiscard]] virtual auto count() const -> std::size_t = 0;
	virtual void set_data(const void* data, std::size_t size, std::size_t offset) = 0;
	virtual void set_data(const void* data, std::size_t size) = 0;

	virtual void set_data(const void* data, std::uint32_t size, std::size_t offset)
	{
		return set_data(data, static_cast<std::size_t>(size), offset);
	};
	virtual void set_data(const void* data, std::uint32_t size) { return set_data(data, static_cast<std::size_t>(size), 0); };

	virtual auto get_raw() -> void* = 0;
	virtual auto get_raw() const -> void* = 0;

	auto invalid() const { return size() == 0; }
};

} // namespace Disarray
