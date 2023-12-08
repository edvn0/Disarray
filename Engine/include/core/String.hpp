#pragma once

#include <string>
#include <string_view>

#include "core/Concepts.hpp"

namespace Disarray::StringUtilities {

namespace Detail {
	auto split_string(const std::string& input, const std::string& delimiter) -> std::vector<std::string>;
}

auto split_string(Stringlike auto input, Stringlike auto delimiter = std::string_view { "," }) { return Detail::split_string(input, delimiter); }

} // namespace Disarray::StringUtilities
