#pragma once

#include <sstream>
#include <string>

#include "fmt/core.h"
#include "util/BitCast.hpp"

namespace Disarray::FormattingUtilities {

template <typename T> inline auto pointer_to_string(T* value) -> std::string
{
	if (value == nullptr) {
		return "nullptr";
	}
	return fmt::format("{}", fmt::ptr(value));
}

} // namespace Disarray::FormattingUtilities
