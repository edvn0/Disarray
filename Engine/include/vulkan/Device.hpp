#pragma once

#include "Forward.hpp"
#include "graphics/Device.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class Device : public Disarray::Device, public PropertySupplier<VkDevice> {
public:
	using base = Disarray::Device;

	Device(Disarray::Window&);
	~Device() override;

	VkQueue get_graphics_queue() const { return graphics; }
	VkQueue get_present_queue() const { return present; }

	VkDevice supply() const override { return device; }

	Disarray::PhysicalDevice& get_physical_device() override { return *physical_device; }
	const Disarray::PhysicalDevice& get_physical_device() const override { return *physical_device; }

private:
	Ref<Disarray::PhysicalDevice> physical_device;

	VkDevice device;
	VkQueue graphics;
	VkQueue present;
};

} // namespace Disarray::Vulkan
