#pragma once

#include <bit>

namespace Disarray {

template <class To> To bit_cast(auto in) { return std::bit_cast<To>(in); }

} // namespace Disarray
