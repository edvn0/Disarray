#pragma once

#include <concepts>
#include <type_traits>

namespace Disarray {

template <typename T>
concept IsNumber = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <typename T>
concept IsEnum = std::is_enum_v<std::remove_cvref_t<T>> || std::is_enum_v<std::unwrap_reference_t<T>>
	|| std::is_enum_v<std::unwrap_reference_t<std::remove_cvref_t<T>>>;

template <typename T, typename... U>
concept AnyOf = (std::same_as<T, U> || ...);

} // namespace Disarray
