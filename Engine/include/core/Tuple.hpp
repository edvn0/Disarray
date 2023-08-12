#pragma once

#include <tuple>

namespace Disarray::Tuple {

	template <class Tup, class Func, std::size_t... Is> constexpr void static_for_impl(Tup&& t, Func&& f, std::index_sequence<Is...>)
	{
		(f(std::integral_constant<std::size_t, Is> {}, std::get<Is>(t)), ...);
	}

	template <class... T, class Func> constexpr void static_for(std::tuple<T...>& t, Func&& f)
	{
		static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)> {});
	}

} // namespace Disarray::Tuple
