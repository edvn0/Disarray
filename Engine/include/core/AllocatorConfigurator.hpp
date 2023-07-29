#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class PhysicalDevice;
	class Instance;

	void initialise_allocator(Ref<Disarray::Device> device, Ref<Disarray::PhysicalDevice> physical_device, Ref<Disarray::Instance> instance);
	void destroy_allocator();

}