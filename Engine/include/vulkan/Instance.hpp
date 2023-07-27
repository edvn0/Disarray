#pragma once

#include "graphics/Instance.hpp"
#include "vulkan/PropertySupplier.hpp"

#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class PhysicalDevice;

	class Instance : public Disarray::Instance, public PropertySupplier<VkInstance> {
	public:
		Instance(std::vector<const char*>&& vec = std::vector<const char*> { "VK_LAYER_KHRONOS_validation" });
		~Instance() override;

		VkInstance get() const override { return instance; }

	private:
		bool check_validation_layer_support() const;
		void setup_debug_messenger();

	private:
		std::vector<const char*> requested_layers {};
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
	};

} // namespace Disarray::Vulkan
