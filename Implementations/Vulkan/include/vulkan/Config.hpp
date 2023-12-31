#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <string_view>

namespace Disarray::Vulkan::Config {

#ifdef IS_RELEASE
static constexpr bool is_debug = false;
#else
static constexpr bool is_debug = true;
#endif

#ifdef USE_VALIDATION_LAYERS
static constexpr auto use_validation_layers = true;
#else
static constexpr auto use_validation_layers = true;
#endif

static constexpr inline auto use_debug_markers = is_debug;

static constexpr std::array<std::string_view, 2> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

static constexpr int max_frames_in_flight = 3;

} // namespace Disarray::Vulkan::Config
