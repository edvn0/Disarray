#pragma once

#ifndef DISARRAY_MACOS
#include <bit>
#endif

namespace Disarray {

#ifndef DISARRAY_MACOS
template <class To> To bit_cast(auto in) { return std::bit_cast<To>(in); }
#else
template<class To> To bit_cast(auto in) { return reinterpret_cast<To>(in); }
#endif

} // namespace Disarray
