#pragma once

#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>

#include "graphics/Instance.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class PhysicalDevice;

class Instance : public Disarray::Instance, public PropertySupplier<VkInstance> {
public:
	Instance(const std::vector<const char*>& vec = { "VK_LAYER_KHRONOS_validation" });
	~Instance() override;

	[[nodiscard]] auto supply() const -> VkInstance override { return instance; }

private:
	bool check_validation_layer_support() const;
	void setup_debug_messenger();

	std::vector<const char*> requested_layers {};
	VkInstance instance {};
	VkDebugUtilsMessengerEXT debug_messenger {};
};

} // namespace Disarray::Vulkan
