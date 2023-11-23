#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/PointerDefinition.hpp"
#include "core/PolymorphicCast.hpp"
#include "core/exceptions/GeneralExceptions.hpp"

namespace Disarray {

template <std::integral T> struct TypeSafeWrapper {
	T value { 0 };

	constexpr explicit TypeSafeWrapper(std::integral auto input) noexcept
		: value(static_cast<T>(input))
	{
	}
	constexpr TypeSafeWrapper() noexcept = default;

	constexpr auto operator+(std::integral auto addend) -> TypeSafeWrapper { return TypeSafeWrapper { value + addend }; }
	constexpr auto operator-(std::integral auto addend) -> TypeSafeWrapper { return TypeSafeWrapper { value - addend }; }
	constexpr auto operator==(const TypeSafeWrapper& other) const -> bool { return value == other.value; }
	constexpr auto operator==(std::integral auto other) const -> bool { return std::cmp_equal(value, other); }
	constexpr auto operator<(std::integral auto other) const -> bool { return std::cmp_less(value, other); }
	constexpr auto operator<=(std::integral auto other) const -> bool { return std::cmp_less_equal(value, other); }
	constexpr auto operator<(const TypeSafeWrapper& other) const -> bool { return std::cmp_less(value, other.value); }
	constexpr auto operator<=(const TypeSafeWrapper& other) const -> bool { return std::cmp_less_equal(value, other.value); }

	auto operator+=(std::integral auto other) -> TypeSafeWrapper&
	{
		value += other;
		return *this;
	}
	auto operator+=(TypeSafeWrapper other) -> TypeSafeWrapper&
	{
		value += other.value;
		return *this;
	}
	auto operator++() -> TypeSafeWrapper&
	{
		value++;
		return *this;
	}
	auto operator++(std::integral auto) -> TypeSafeWrapper
	{
		TypeSafeWrapper old = *this;
		operator++();
		return old;
	}
	auto operator--() -> TypeSafeWrapper&
	{
		value++;
		return *this;
	}
	auto operator--(std::integral auto) -> TypeSafeWrapper
	{
		TypeSafeWrapper old = *this;
		operator--();
		return old;
	}

	explicit operator T() const { return value; }
};

using UnkownData = void*;
#ifdef CUSTOM_PREFER_IRC
template <class T, class... Args> inline auto make_ref(Args&&... args) -> Ref<T>
{
	return ReferenceCounted<T> { new T { std::forward<Args>(args)... } };
}
#else
template <class T, class... Args> Ref<T> make_ref(Args&&... args) { return std::shared_ptr<T> { new T { std::forward<Args>(args)... } }; }
#endif

template <class T, class D = DefaultDelete<T>, class... Args> inline auto make_scope(Args&&... args) -> Scope<T, D>
{
	return Scope<T, D> { new T { std::forward<Args>(args)... }, D {} };
}

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
	requires(std::is_base_of_v<From, To> && requires(To& t) { t.supply(); })
auto supply_cast(From& obj) -> decltype(auto)
{
	return polymorphic_cast<To>(obj).supply();
}

template <class To, class From>
	requires(std::is_base_of_v<From, To> && requires(const To& t) { t.supply(); })
auto supply_cast(const From& obj) -> decltype(auto)
{
	return polymorphic_cast<To>(obj).supply();
}

[[noreturn]] inline auto unreachable(std::string_view info = "Reached unreachable code") { throw UnreachableException { info }; }

} // namespace Disarray

template <std::integral T> struct fmt::formatter<Disarray::TypeSafeWrapper<T>> : fmt::formatter<std::string_view> {
	auto format(const Disarray::TypeSafeWrapper<T>& format, format_context& ctx) -> decltype(ctx.out());
};

template <std::integral T> struct std::hash<Disarray::TypeSafeWrapper<T>> {
	auto operator()(const Disarray::TypeSafeWrapper<T>& index) const -> std::size_t
	{
		std::hash<T> hasher {};
		return hasher(index.value);
	}
};
