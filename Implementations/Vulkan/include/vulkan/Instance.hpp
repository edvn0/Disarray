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
	explicit Instance(bool runtime_requested_validation = true, const std::vector<const char*>& = { "VK_LAYER_KHRONOS_validation" });
	~Instance() override;

	[[nodiscard]] auto supply() const -> VkInstance override { return instance; }

private:
	[[nodiscard]] auto check_validation_layer_support() const -> bool;
	void setup_debug_messenger();

	bool use_validation_layers { true };
	std::vector<const char*> requested_layers {};
	VkInstance instance {};
	VkDebugUtilsMessengerEXT debug_messenger {};
};

} // namespace Disarray::Vulkan
