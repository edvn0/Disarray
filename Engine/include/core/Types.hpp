#pragma once

#include "core/PointerDefinition.hpp"
#include "core/PolymorphicCast.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace Disarray {

	using UnkownData = void*;
#ifdef CUSTOM_PREFER_IRC
	template <class T, class... Args> inline Ref<T> make_ref(Args&&... args) { return ReferenceCounted<T> { new T { std::forward<Args>(args)... } }; }
#else
	template <class T, class... Args> Ref<T> make_ref(Args&&... args) { return std::shared_ptr<T> { new T { std::forward<Args>(args)... } }; }
#endif

	template <class T, class... Args> inline Scope<T> make_scope(Args&&... args) { return Scope<T> { new T { std::forward<Args>(args)... } }; }

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	decltype(auto) cast_to(From&& obj)
	{
		return polymorphic_cast<To&>(obj);
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	decltype(auto) cast_to(const From& obj)
	{
		return polymorphic_cast<const To>(obj);
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	Ref<To> cast_to(const Ref<From>& obj)
	{
		return polymorphic_cast<To>(obj);
	}

	template <class To, class From>
		requires(std::is_base_of_v<From, To>)
	Ref<To> cast_to(Ref<From>&& obj)
	{
		return polymorphic_cast<To>(obj);
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

	[[noreturn]] inline auto unreachable() { throw std::runtime_error("Reached unreachable code."); }

} // namespace Disarray
