#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class Instance;

	void initialise_allocator(Disarray::Device& device, Disarray::Instance& instance);
	void destroy_allocator();

} // namespace Disarray
