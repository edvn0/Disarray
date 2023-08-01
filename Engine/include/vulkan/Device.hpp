#pragma once

#include "Forward.hpp"
#include "graphics/Device.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class Device : public Disarray::Device, public PropertySupplier<VkDevice> {
	public:
		Device(Disarray::Window&);
		~Device() override;

		VkQueue get_graphics_queue() { return graphics; }
		VkQueue get_present_queue() { return present; }

		VkDevice supply() const override { return device; }

		Disarray::PhysicalDevice& get_physical_device() override { return *physical_device; }

	private:
		Ref<Disarray::PhysicalDevice> physical_device;

		VkDevice device;
		VkQueue graphics;
		VkQueue present;
	};

} // namespace Disarray::Vulkan
