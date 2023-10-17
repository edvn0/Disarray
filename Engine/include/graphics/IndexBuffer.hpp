#pragma once

#include "graphics/BufferProperties.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Types.hpp"

namespace Disarray {

class Device;
class Swapchain;
class PhysicalDevice;

class IndexBuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(IndexBuffer, BufferProperties)
public:
	[[nodiscard]] virtual auto size() const -> std::size_t = 0;
	virtual void set_data(const void* data, std::size_t size, std::size_t offset) = 0;
	virtual void set_data(const void* data, std::uint32_t size, std::size_t offset) { return set_data(data, static_cast<std::size_t>(size), offset); };
};

} // namespace Disarray
