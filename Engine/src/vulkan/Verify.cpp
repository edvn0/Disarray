#include "DisarrayPCH.hpp"

#include "vulkan/Verify.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

#include <magic_enum.hpp>

namespace Disarray::Vulkan {

std::string_view from_vulkan_result(VkResult result) { return magic_enum::enum_name(result); }

void verify(VkResult result)
{
	if (result != VK_SUCCESS) {
		throw ResultException(from_vulkan_result(result));
	}
}

} // namespace Disarray::Vulkan
