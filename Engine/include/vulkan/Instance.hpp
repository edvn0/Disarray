#pragma once

#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class Instance {
	public:
		Instance(std::vector<const char*>&& vec = std::vector<const char*> { "VK_LAYER_KHRONOS_validation" });
		~Instance();

	private:
		bool check_validation_layer_support() const;
		void setup_debug_messenger();
		void load_physical_device();

	private:
		std::vector<const char*> requested_layers {};
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		VkPhysicalDevice physical_device;
	};

} // namespace Disarray::Vulkan
