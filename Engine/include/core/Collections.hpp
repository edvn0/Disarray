#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/Concepts.hpp"
#include "core/Hashes.hpp"
#include "core/PointerDefinition.hpp"

#ifdef DISARRAY_WINDOWS
#include <execution>
#endif

namespace Disarray::Collections {

template <class T> class DefaultAllocator : public std::allocator<T> {
public:
	DefaultAllocator() noexcept = default;
	~DefaultAllocator() = default;
	DefaultAllocator(DefaultAllocator&&) noexcept = default;
	auto operator=(const DefaultAllocator&) -> DefaultAllocator& = default;
	auto operator=(DefaultAllocator&&) noexcept -> DefaultAllocator& = default;
	DefaultAllocator(const DefaultAllocator& other) noexcept = default;

	template <class Other> DefaultAllocator(const DefaultAllocator<Other>& other) noexcept {};
	template <class Other> using rebind = DefaultAllocator<Other>;

	void deallocate(T* const ptr, const size_t count) { std::allocator<T>::deallocate(ptr, count); };
	auto allocate(const size_t count) -> T* { return std::allocator<T>::allocate(count); };
};

namespace Detail {
	template <class Key, class Value> using PairAllocator = DefaultAllocator<std::pair<const Key, Value>>;

	template <class K, class V>
		requires(AnyOf<K, const char*, std::string_view, std::string>)
	using StringlikeUnorderedMap = std::unordered_map<K, V, StringHash, std::equal_to<>, Detail::PairAllocator<K, V>>;
} // namespace Detail

template <class Value> using StringViewMap = Detail::StringlikeUnorderedMap<std::string_view, Value>;
template <class Value> using StringMap = Detail::StringlikeUnorderedMap<std::string, Value>;
template <class Value> using ScopedStringViewMap = Detail::StringlikeUnorderedMap<std::string_view, Scope<Value>>;
template <class Value> using ScopedStringMap = Detail::StringlikeUnorderedMap<std::string, Scope<Value>>;
template <class Value> using ReferencedStringViewMap = Detail::StringlikeUnorderedMap<std::string_view, Ref<Value>>;
template <class Value> using ReferencedStringMap = Detail::StringlikeUnorderedMap<std::string, Ref<Value>>;
using StringViewSet = std::unordered_set<std::string_view, StringHash, std::equal_to<>, DefaultAllocator<std::string_view>>;
using StringSet = std::unordered_set<std::string, StringHash, std::equal_to<>, DefaultAllocator<std::string>>;

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
	using ReturnType = decltype(func(*std::begin(collection)));
	std::vector<ReturnType> output;
	const auto cbegin = std::cbegin(collection);
	const auto cend = std::cend(collection);
	const auto elements = std::distance(cbegin, cend);
	output.reserve(elements);

	for (auto it = cbegin; it != cend;) {
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
