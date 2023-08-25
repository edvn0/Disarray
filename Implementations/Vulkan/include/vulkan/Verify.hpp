#pragma once

#include <vulkan/vulkan.h>

#include <exception>
#include <string_view>

namespace Disarray::Vulkan {

void verify(VkResult result);

} // namespace Disarray::Vulkan
