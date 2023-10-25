#pragma once

#include <span>

#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"
#include "graphics/BufferProperties.hpp"

namespace Disarray {

class Device;
class Swapchain;
class PhysicalDevice;

class StorageBuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(StorageBuffer, BufferProperties)
public:
	[[nodiscard]] virtual auto size() const -> std::size_t = 0;
	[[nodiscard]] virtual auto count() const -> std::size_t = 0;
	virtual void set_data(const void* data, std::size_t size, std::size_t offset) = 0;
	virtual void set_data(const void* data, std::uint32_t size, std::size_t offset)
	{
		return set_data(data, static_cast<std::size_t>(size), offset);
	};

	template <class T> auto get_mutable() { return std::span<T>(static_cast<T*>(get_raw()), count()); }

protected:
	virtual auto get_raw() -> void* = 0;
};

} // namespace Disarray
