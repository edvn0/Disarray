#pragma once

#include <concepts>
#include <type_traits>

namespace Disarray {

	template <typename T>
	concept IsNumber = std::is_floating_point_v<T> || std::is_integral_v<T>;

	template <typename T, typename... U>
	concept AnyOf = (std::same_as<T, U> || ...);

} // namespace Disarray
