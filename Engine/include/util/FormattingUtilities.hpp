#pragma once

#include <fmt/core.h>

#include <sstream>
#include <string>

#include "util/BitCast.hpp"

namespace Disarray::FormattingUtilities {

namespace Detail {

	auto format_pointer(const void* ptr) -> const void*;

	template <typename T> auto format_pointer(T* ptr) -> const void* { return format_pointer(static_cast<const void*>(ptr)); }

} // namespace Detail

template <typename T> inline auto pointer_to_string(T* value) -> std::string
{
	if (value == nullptr) {
		return "nullptr";
	}
	return fmt::format("{}", Detail::format_pointer<T>(value));
}

} // namespace Disarray::FormattingUtilities
