#pragma once

#include "graphics/Device.hpp"
#include "vulkan/PropertySupplier.hpp"

#include <vulkan/vulkan.hpp>

namespace Disarray {
	class Surface;
	class PhysicalDevice;
} // namespace Disarray

namespace Disarray::Vulkan {

	class Device : public Disarray::Device, public PropertySupplier<VkDevice> {
	public:
		Device(Ref<Disarray::PhysicalDevice>, Ref<Disarray::Surface>);
		~Device() override;

		VkQueue get_graphics_queue() { return graphics; }
		VkQueue get_present_queue() { return present; }

		VkDevice supply() const override { return device; }

	private:
		VkDevice device;
		VkQueue graphics;
		VkQueue present;
	};

} // namespace Disarray::Vulkan
