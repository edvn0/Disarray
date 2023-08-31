#pragma once

#include <concepts>
#include <type_traits>

namespace Disarray {

template <typename T>
concept IsNumber = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <typename T>
concept IsEnum = std::is_enum_v<T> || std::is_enum_v<std::remove_reference_t<T>> || std::is_enum_v<typename T::type>;

template <typename T, typename... U>
concept AnyOf = (std::same_as<T, U> || ...);

} // namespace Disarray
