#pragma once

#include <concepts>
#include <type_traits>

namespace Disarray {

template <class Input, class Func, class ReturnType>
concept SpecifiedReturnTypeFunc = requires(Func&& func, Input& input) {
	{
		func(input)
	} -> std::same_as<ReturnType>;
};

template <std::size_t T, std::size_t Min, std::size_t Max>
concept InRange = (T > Min && T <= Max);

template <std::size_t T, std::size_t Value>
concept InSingleRange = (T > -Value && T <= Value);

template <typename T>
concept IsNumber = std::is_floating_point_v<T> || std::is_integral_v<T>;

template <typename T>
concept IsEnum = std::is_enum_v<T> || std::is_enum_v<std::remove_reference_t<T>> || std::is_enum_v<typename T::type>;

template <typename T, typename... U>
concept AnyOf = (std::same_as<T, U> || ...);

template <class T>
concept Pathlike = AnyOf<T, std::string, std::string_view, std::filesystem::path, const char*>;

} // namespace Disarray
