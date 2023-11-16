#pragma once

#include "Forward.hpp"
#include "graphics/Device.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class Device : public Disarray::Device, public PropertySupplier<VkDevice> {
public:
	explicit Device(Disarray::Window&);
	~Device() override;

	auto get_graphics_queue() const -> VkQueue { return graphics; }
	auto get_present_queue() const -> VkQueue { return present; }

	auto supply() const -> VkDevice override { return device; }
	auto supply() -> VkDevice override { return device; }

	auto get_physical_device() -> Disarray::PhysicalDevice& override { return *physical_device; }
	auto get_physical_device() const -> const Disarray::PhysicalDevice& override { return *physical_device; }

private:
	Ref<Disarray::PhysicalDevice> physical_device;

	VkDevice device {};
	VkQueue graphics {};
	VkQueue present {};
};

} // namespace Disarray::Vulkan
