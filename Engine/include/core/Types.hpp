#pragma once

#include "core/ReferenceCounted.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace Disarray {

	using UnkownData = void*;

	template <class T> using Ref = ReferenceCounted<T>;
	template <class T, class... Args> Ref<T> make_ref(Args&&... args) { return ReferenceCounted<T> { new T { std::forward<Args>(args)... } }; }
	template <class T, class... Args> Ref<T> make_ref_inplace(Args&&... args) { return ReferenceCounted<T> { std::forward<Args>(args)... }; }

	template <class T> using Scope = std::unique_ptr<T>;
	template <class T, class... Args> Scope<T> make_scope(Args&&... args) { return Scope<T> { new T { std::forward<Args>(args)... } }; }

	template <class T>
	concept ScopeOrRef = std::is_same_v<T, Scope<T>> || std::is_same_v<T, Ref<T>>;

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

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	decltype(auto) cast_to(From&& obj)
	{
		return polymorphic_cast<To&>(std::forward<From>(obj));
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	decltype(auto) cast_to(const From& obj)
	{
		return polymorphic_cast<const To>(obj);
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	decltype(auto) cast_to(From& obj)
	{
		return polymorphic_cast<To>(obj);
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To> && requires(To t) { t.supply(); })
	decltype(auto) supply_cast(From&& obj)
	{
		return polymorphic_cast<To>(std::forward<From>(obj)).supply();
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To> && requires(To t) { t.supply(); })
	decltype(auto) supply_cast(From& obj)
	{
		return polymorphic_cast<To>(obj).supply();
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To> && requires(To t) { t.supply(); })
	decltype(auto) supply_cast(const From& obj)
	{
		return polymorphic_cast<To>(obj).supply();
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To> && requires(To t) { t.supply_indexed(); })
	decltype(auto) indexed_supply_cast(From&& obj)
	{
		return polymorphic_cast<To>(std::forward<From>(obj)).supply_indexed();
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To> && requires(To t) { t.supply_indexed(); })
	decltype(auto) indexed_supply_cast(From& obj)
	{
		return polymorphic_cast<To>(obj).supply_indexed();
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To> && requires(To t) { t.supply_indexed(); })
	decltype(auto) indexed_supply_cast(const From& obj)
	{
		return polymorphic_cast<To>(obj).supply_indexed();
	}

	[[noreturn]] inline auto unreachable() { throw std::runtime_error("Reached unreachable code."); }

} // namespace Disarray
