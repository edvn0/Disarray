#include "DisarrayPCH.hpp"

#include "core/Log.hpp"
#include "vulkan/exceptions/VulkanExceptions.hpp"

namespace Disarray {

ResultException::ResultException(std::string_view msg)
	: BaseException("ResultException", msg)
{
	Log::error("ResultException", "Exception:{}", msg);
}

} // namespace Disarray
