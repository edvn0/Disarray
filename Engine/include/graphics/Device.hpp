#pragma once

#include "Forward.hpp"
#include "core/Types.hpp"

namespace Disarray {

class Window;

class Device {
public:
	virtual ~Device() = default;
	virtual auto get_physical_device() -> Disarray::PhysicalDevice& = 0;
	[[nodiscard]] virtual auto get_physical_device() const -> const Disarray::PhysicalDevice& = 0;
	static auto construct(Disarray::Window&) -> Scope<Device>;
};

} // namespace Disarray
