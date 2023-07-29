#pragma once

#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"
#include "graphics/Surface.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class PhysicalDevice : public Disarray::PhysicalDevice, public PropertySupplier<VkPhysicalDevice> {
	public:
		explicit PhysicalDevice(Ref<Disarray::Instance>, Ref<Disarray::Surface>);

		Ref<Disarray::QueueFamilyIndex> get_queue_family_indexes() override { return queue_family_index; }

		VkPhysicalDevice supply() const override { return physical_device; }

	private:
		VkPhysicalDevice physical_device;
		Ref<Disarray::QueueFamilyIndex> queue_family_index;
	};

} // namespace Disarray::Vulkan
