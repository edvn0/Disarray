#pragma once

#include <exception>
#include <string_view>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

void verify(VkResult result);

} // namespace Disarray::Vulkan
