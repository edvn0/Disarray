#pragma once

#include <cstddef>
#include <cstdint>

#include "RendererProperties.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

class UniformBuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(UniformBuffer, BufferProperties)
public:
	[[nodiscard]] virtual auto size() const -> std::size_t = 0;
	[[nodiscard]] virtual auto count() const -> std::size_t = 0;
	[[nodiscard]] virtual auto get_binding() const -> DescriptorBinding = 0;
	virtual void set_data(const void* data, std::size_t size, std::size_t offset) = 0;
	virtual void set_data(const void* data, std::size_t size) = 0;

	virtual void set_data(const void* data, std::uint32_t size, std::size_t offset)
	{
		return set_data(data, static_cast<std::size_t>(size), offset);
	};
	virtual void set_data(const void* data, std::uint32_t size) { return set_data(data, static_cast<std::size_t>(size), 0); };

	virtual auto get_raw() -> void* = 0;
	virtual auto get_raw() const -> void* = 0;

	template <class T> auto get_data() -> T& { return *Disarray::bit_cast<T*>(get_raw()); };

	template <class T>
		requires(sizeof(T) > 0)
	void set_data(T* data)
	{
		set_data<T>(data, 1);
	};

	template <class T>
		requires(sizeof(T) > 0)
	void set_data(T* data, std::size_t count)
	{
		set_data(Disarray::bit_cast<const void*>(data), count * sizeof(T));
	};
};

} // namespace Disarray
