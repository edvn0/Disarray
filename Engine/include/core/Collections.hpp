#pragma once

#include <algorithm>
#include <array>
#include <iterator>
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
	{
		*std::begin(collection)
	};
};

template <class T>
concept ConstIterable = Iterable<T> && requires(T collection) {
	{
		std::cbegin(collection)
	};
	{
		std::cend(collection)
	};
};

template <class T, std::size_t BatchSize> auto split_into_batches(Iterable auto& collection)
{
	using Vector = std::vector<T>;

	std::vector<Vector> rtn;
	auto cbegin = std::cbegin(collection);
	const auto cend = std::cend(collection);

	while (cbegin != cend) {
		Vector vector {};
		vector.reserve(BatchSize);

		std::back_insert_iterator<Vector> inserter(vector);
		const auto distance = static_cast<std::size_t>(std::distance(cbegin, cend));
		const auto num_to_copy = std::min(distance, BatchSize);
		std::copy(cbegin, cbegin + num_to_copy, inserter);
		rtn.push_back(std::move(vector));
		std::advance(cbegin, num_to_copy);
	}

	return rtn;
}

constexpr inline auto for_each(Iterable auto& collection, auto&& func) { std::for_each(std::begin(collection), std::end(collection), func); }
constexpr inline auto remove_if(Iterable auto& collection, auto&& predicate)
{
	auto iterator = std::begin(collection);
	auto end = std::end(collection);
	for (; iterator != end;) {
		if (predicate(*iterator)) {
			iterator = collection.erase(iterator);
		} else {
			++iterator;
		}
	}
}
constexpr inline auto map(Iterable auto& collection, auto&& func)
{
	std::vector<decltype(func(*std::begin(collection)))> output;
	const auto elements = std::distance(std::begin(collection), std::end(collection));
	output.reserve(elements);

	for (auto it = std::begin(collection); it != std::end(collection);) {
		output.push_back(func(*it));
		it++;
	}

	return output;
}

#ifdef DISARRAY_WINDOWS
constexpr inline void parallel_for_each(Iterable auto& collection, auto&& func)
{
	std::for_each(std::execution::par, std::begin(collection), std::end(collection), func);
}
#else
constexpr inline void parallel_for_each(Iterable auto& collection, auto&& func) { std::for_each(std::begin(collection), std::end(collection), func); }
#endif

} // namespace Disarray::Collections
