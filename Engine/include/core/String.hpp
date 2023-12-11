#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "core/Concepts.hpp"

namespace Disarray::StringUtilities {

namespace Detail {
	auto split_string(const std::string& input, const std::string& delimiter, bool correct_slashes) -> std::vector<std::string>;
	auto human_readable_size(std::size_t size) -> std::string;
} // namespace Detail

auto split_string(Stringlike auto input, Stringlike auto delimiter = std::string_view { "," }, bool correct_slashes = false)
{
	return Detail::split_string(input, delimiter, correct_slashes);
}

inline auto human_readable_size(std::size_t size) -> std::string { return Detail::human_readable_size(size); }

} // namespace Disarray::StringUtilities
