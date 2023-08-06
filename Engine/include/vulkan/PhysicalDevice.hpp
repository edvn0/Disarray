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

		Disarray::QueueFamilyIndex& get_queue_family_indexes() override { return *queue_family_index; }

		VkPhysicalDevice supply() const override { return physical_device; }

		VkPhysicalDeviceLimits get_limits() const { return device_properties.limits; }
		SampleCount get_sample_count() const { return samples; }

	private:
		VkPhysicalDevice physical_device;
		Ref<Disarray::QueueFamilyIndex> queue_family_index;
		VkPhysicalDeviceProperties device_properties;
		SampleCount samples { SampleCount::ONE };
	};

} // namespace Disarray::Vulkan
