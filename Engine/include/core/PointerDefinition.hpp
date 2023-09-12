#pragma once

#include <memory>

#include "core/ReferenceCounted.hpp"

namespace Disarray {
#define CUSTOM_PREFER_IRC
#ifdef CUSTOM_PREFER_IRC
template <class T> using Ref = ReferenceCounted<T>;
#else
template <class T> using Ref = std::shared_ptr<T>;
#endif

template <class T> struct PimplDeleter {
	constexpr PimplDeleter() noexcept = default;
	auto operator()(T* ptr) noexcept -> void;
};

template <class T> struct DefaultDelete {
	constexpr DefaultDelete() noexcept = default;

	template <class Other>
	constexpr DefaultDelete(const DefaultDelete<Other>&) noexcept
		requires(std::is_convertible_v<Other*, T*>)
	{
	}

	constexpr void operator()(T* ptr) const noexcept
	{
		static_assert(0 < sizeof(T), "can't delete an incomplete type");
		delete ptr;
	}
};

template <class T, class D = DefaultDelete<T>> using Scope = std::unique_ptr<T, D>;

} // namespace Disarray
