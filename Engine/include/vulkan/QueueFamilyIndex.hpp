#pragma once

#include "core/Types.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"

#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class QueueFamilyIndex : public Disarray::QueueFamilyIndex {
	public:
		~QueueFamilyIndex() override = default;
		explicit QueueFamilyIndex(Disarray::PhysicalDevice&, Disarray::Surface&);
		explicit QueueFamilyIndex(VkPhysicalDevice, Disarray::Surface&);
	};

} // namespace Disarray::Vulkan
