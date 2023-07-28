#pragma once

#include <array>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan::Config {

#ifdef IS_DEBUG
	static constexpr bool is_debug = true;
#else
	static constexpr bool is_debug = false;
#endif

	static constexpr auto use_validation_layers = is_debug;

	static std::array<const char*, 1> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	static constexpr int max_frames_in_flight = 3;

} // namespace Disarray::Vulkan::Config