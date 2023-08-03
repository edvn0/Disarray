#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class PhysicalDevice;

	void initialise_debug_applications(Disarray::Device& device);

	void destroy_debug_applications();

} // namespace Disarray
