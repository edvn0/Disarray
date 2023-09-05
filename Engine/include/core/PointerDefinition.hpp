#pragma once

#include <memory>

#include "core/ReferenceCounted.hpp"

namespace Disarray {
#define CUSTOM_PREFER_IRC
#ifdef CUSTOM_PREFER_IRC
template <class T> using Ref = ReferenceCounted<T>;
#else
template <class T> using Ref = std::shared_ptr<T>;
#endif
template <class T, class D = std::default_delete<T>> using Scope = std::unique_ptr<T, D>;

} // namespace Disarray
