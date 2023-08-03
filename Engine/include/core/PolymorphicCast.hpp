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
	Ref<To> polymorphic_cast(const Ref<From>& object)
	{
		if constexpr (std::is_same_v<Ref<From>, std::shared_ptr<From>>) {
			return std::dynamic_pointer_cast<To>(object);
		} else {
			return object.template as<To>();
		}
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
