#pragma once

#include "graphics/PhysicalDevice.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class PhysicalDevice : public Disarray::PhysicalDevice, public PropertySupplier<VkPhysicalDevice> {
	public:
		explicit PhysicalDevice(Ref<Disarray::Instance>, Ref<Disarray::Surface>);

		VkPhysicalDevice supply() const override { return physical_device; }

	private:
		VkPhysicalDevice physical_device;
	};

} // namespace Disarray::Vulkan
