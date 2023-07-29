#pragma once

#include "graphics/Device.hpp"

#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class Device : public Disarray::Device, public PropertySupplier<VkDevice> {
	public:
		Device(Ref<Disarray::PhysicalDevice>);
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
