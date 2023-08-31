#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "core/PointerDefinition.hpp"
#include "core/PolymorphicCast.hpp"
#include "core/exceptions/GeneralExceptions.hpp"

namespace Disarray {

using UnkownData = void*;
#ifdef CUSTOM_PREFER_IRC
template <class T, class... Args> inline auto make_ref(Args&&... args) -> Ref<T>
{
	return ReferenceCounted<T> { new T { std::forward<Args>(args)... } };
}
#else
template <class T, class... Args> Ref<T> make_ref(Args&&... args) { return std::shared_ptr<T> { new T { std::forward<Args>(args)... } }; }
#endif

template <class T, class... Args> inline auto make_scope(Args&&... args) -> Scope<T> { return Scope<T> { new T { std::forward<Args>(args)... } }; }

template <class To, class From>
	requires(std::is_base_of_v<From, To>)
auto cast_to(From&& obj) -> decltype(auto)
{
	return polymorphic_cast<To&>(std::forward<From>(obj));
}

template <class To, class From>
	requires(std::is_base_of_v<From, To>)
auto cast_to(const From& obj) -> decltype(auto)
{
	return polymorphic_cast<const To>(obj);
}

template <class To, class From>
	requires(std::is_base_of_v<From, To>)
auto cast_to(From& obj) -> decltype(auto)
{
	return polymorphic_cast<To>(obj);
}

template <class To, class From>
	requires(std::is_base_of_v<From, To> && requires(To t) { t.supply(); })
auto supply_cast(From&& obj) -> decltype(auto)
{
	return polymorphic_cast<To>(std::forward<From>(obj)).supply();
}

template <class To, class From>
	requires(std::is_base_of_v<From, To> && requires(To t) { t.supply(); })
auto supply_cast(From& obj) -> decltype(auto)
{
	return polymorphic_cast<To>(obj).supply();
}

template <class To, class From>
	requires(std::is_base_of_v<From, To> && requires(To t) { t.supply(); })
auto supply_cast(const From& obj) -> decltype(auto)
{
	return polymorphic_cast<To>(obj).supply();
}

[[noreturn]] inline auto unreachable(std::string_view info = "Reached unreachable code") { throw UnreachableException { info }; }

} // namespace Disarray
