#pragma once

#include "core/PointerDefinition.hpp"

#include <concepts>
#include <memory>

namespace Disarray {

template <class To, class From>
	requires(std::is_base_of_v<From, To>)
decltype(auto) polymorphic_cast(From&& object)
{
#ifdef IS_DEBUG
	return dynamic_cast<To&>(std::forward<From>(object));
#else
	return static_cast<To&>(std::forward<From>(object));
#endif
}

template <class To, class From>
	requires(std::is_base_of_v<From, To>)
decltype(auto) polymorphic_cast(const From& object)
{
#ifdef IS_DEBUG
	return dynamic_cast<const To&>(object);
#else
	return static_cast<const To&>(object);
#endif
}

template <class To, class From>
	requires(std::is_base_of_v<From, To>)
decltype(auto) polymorphic_cast(From& object)
{
#ifdef IS_DEBUG
	return dynamic_cast<To&>(object);
#else
	return static_cast<To&>(object);
#endif
}
} // namespace Disarray
