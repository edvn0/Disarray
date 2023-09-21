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

#ifdef DISARRAY_WINDOWS
static constexpr auto is_windows = true;
#else
static constexpr auto is_windows = false;
#endif

static constexpr auto use_validation_layers = is_debug && is_windows;

static constexpr std::array<std::string_view, 2> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

static constexpr int max_frames_in_flight = 3;

} // namespace Disarray::Vulkan::Config
