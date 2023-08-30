#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/Hashes.hpp"

#ifdef DISARRAY_WINDOWS
#include <execution>
#endif

namespace Disarray::Collections {

template <class Value> using StringViewMap = std::unordered_map<std::string_view, Value, StringHash, std::equal_to<>>;
template <class Value> using StringMap = std::unordered_map<std::string, Value, StringHash, std::equal_to<>>;
using StringViewSet = std::unordered_set<std::string_view, StringHash>;
using StringSet = std::unordered_set<std::string, StringHash>;

template <class T>
concept Iterable = requires(T collection) {
	{
		std::begin(collection)
	};
	{
		std::end(collection)
	};
};

template <class T, std::size_t BatchSize> auto split_into_batches(const std::vector<T>& collection)
{
	using type = T;
	std::vector<std::vector<type>> out {};
	for (auto it = std::begin(collection); it != std::end(collection);) {
		std::size_t added = 0;
		std::vector<type> sub_vector {};
		sub_vector.reserve(BatchSize);
		while (added++ < BatchSize) {
			sub_vector.push_back(*it++);
		}
		out.push_back(sub_vector);
		it++;
	}
	return out;
}

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
