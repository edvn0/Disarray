#pragma once

#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef DISARRAY_WINDOWS
#include <execution>
#endif

namespace Disarray::Collections {

template <class T>
concept Iterable = requires(T t) {
	{
		std::begin(t)
	};
	{
		std::end(t)
	};
};

constexpr inline void for_each(Iterable auto& collection, auto&& func) { std::for_each(std::begin(collection), std::end(collection), func); }

#ifdef DISARRAY_WINDOWS
constexpr inline void parallel_for_each(Iterable auto& collection, auto&& func)
{
	std::for_each(std::execution::par, std::begin(collection), std::end(collection), func);
}
#else
constexpr inline void parallel_for_each(Iterable auto& collection, auto&& func) { std::for_each(std::begin(collection), std::end(collection), func); }
#endif

} // namespace Disarray::Collections
