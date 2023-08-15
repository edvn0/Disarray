#pragma once

#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

} // namespace Disarray::Collections
