#pragma once

#include "graphics/ImageProperties.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class PhysicalDevice : public Disarray::PhysicalDevice, public PropertySupplier<VkPhysicalDevice> {
public:
	explicit PhysicalDevice(Disarray::Instance&, Disarray::Surface&);
	~PhysicalDevice() override;

	void recreate(bool, const Disarray::Extent&) override { }
	void force_recreation() override { }

	auto get_queue_family_indexes() -> Disarray::QueueFamilyIndex& override { return *queue_family_index; }
	auto get_queue_family_indexes() const -> const Disarray::QueueFamilyIndex& override { return *queue_family_index; }

	auto supply() const -> VkPhysicalDevice override { return physical_device; }

	auto get_limits() const -> VkPhysicalDeviceLimits { return device_properties.limits; }
	auto get_sample_count() const -> SampleCount { return samples; }

private:
	VkPhysicalDevice physical_device;
	Ref<Disarray::QueueFamilyIndex> queue_family_index;
	VkPhysicalDeviceProperties device_properties {};
	SampleCount samples { SampleCount::One };
};

} // namespace Disarray::Vulkan
