#pragma once

#include <exception>
#include <string_view>
#include <vulkan/vulkan.h>

namespace Disarray::Vulkan {

	class VerificationException : public std::exception {
	public:
		VerificationException(std::string_view msg)
			: message(msg) {};

		const char* what() const override { return message.data(); }

	private:
		std::string_view message {};
	};

	static void verify(VkResult result)
	{
		if (result != VK_SUCCESS) {
			throw VerificationException("failed to create instance!");
		}
	}

} // namespace Disarray::Vulkan
