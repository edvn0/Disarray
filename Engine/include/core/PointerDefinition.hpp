#pragma once

#include "core/ReferenceCounted.hpp"

#include <memory>

namespace Disarray {
#define CUSTOM_PREFER_IRC
#ifdef CUSTOM_PREFER_IRC
template <class T> using Ref = ReferenceCounted<T>;
#else
template <class T> using Ref = std::shared_ptr<T>;
#endif
template <class T> using Scope = std::unique_ptr<T>;

} // namespace Disarray
