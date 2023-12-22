#include "DisarrayPCH.hpp"

#include <magic_enum.hpp>

#include <string_view>

#include "vulkan/Verify.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

namespace Disarray::Vulkan {

auto from_vulkan_result(VkResult result) -> std::string_view { return magic_enum::enum_name(result); }

void verify(VkResult result)
{
	if (result != VK_SUCCESS) {
		throw ResultException(from_vulkan_result(result));
	}
}

} // namespace Disarray::Vulkan
