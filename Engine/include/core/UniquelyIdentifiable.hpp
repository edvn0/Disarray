#pragma once

#include <concepts>
#include <cstddef>

namespace Disarray {

	using Identifier = std::size_t;

	template <class Derived>
	struct UniquelyIdentifiable {
		template<class T = Derived>
		Identifier hash() { return static_cast<T&>(*this).hash_impl(); }
	};

} // namespace Disarray