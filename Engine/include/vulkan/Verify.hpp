#pragma once

#include <exception>
#include <string_view>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class VerificationException : public std::exception {
	public:
		VerificationException(std::string_view msg)
			: message(msg)
		{
		}

		const char* what() const noexcept override { return message.data(); }

	private:
		std::string_view message {};
	};

	std::string_view from_vulkan_result(VkResult result);
	void verify(VkResult result);

} // namespace Disarray::Vulkan
