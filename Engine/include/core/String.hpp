#pragma once

#include <string>
#include <string_view>

#include "core/Concepts.hpp"

namespace Disarray::StringUtilities {

namespace Detail {
	auto split_string(const std::string& input, const std::string& delimiter, bool correct_slashes) -> std::vector<std::string>;
}

auto split_string(Stringlike auto input, Stringlike auto delimiter = std::string_view { "," }, bool correct_slashes = false)
{
	return Detail::split_string(input, delimiter, correct_slashes);
}

} // namespace Disarray::StringUtilities
