#pragma once

#include <sstream>
#include <string>

namespace Disarray::FormattingUtilities {

	template <typename T> static auto pointer_to_string(T* t) -> std::string
	{
		if (t == nullptr)
			return "nullptr";
		static std::string prefix = "0x";
		std::ostringstream address;
		address << (static_cast<const void*>(t));
		std::string name = address.str();
		return prefix + name;
	}

} // namespace Disarray::FormattingUtilities
