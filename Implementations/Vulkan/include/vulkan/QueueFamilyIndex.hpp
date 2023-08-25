#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <stdexcept>
#include <vector>

#include "core/Types.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/QueueFamilyIndex.hpp"

namespace Disarray::Vulkan {

class QueueFamilyIndex : public Disarray::QueueFamilyIndex {
public:
	~QueueFamilyIndex() override = default;
	explicit QueueFamilyIndex(Disarray::PhysicalDevice&, Disarray::Surface&);
	explicit QueueFamilyIndex(VkPhysicalDevice, Disarray::Surface&);
};

} // namespace Disarray::Vulkan
