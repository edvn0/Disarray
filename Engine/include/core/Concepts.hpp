#pragma once

#include <concepts>
#include <type_traits>

namespace Disarray {

	template <typename T>
	concept IsNumber = std::is_floating_point_v<T> || std::is_integral_v<T>;

	template <class U, class... T>
	concept AnyOf = (std::is_same_v<U, T> || ...);

} // namespace Disarray
