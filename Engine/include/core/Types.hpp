#pragma once

#include <cstdint>
#include <memory>

namespace Disarray {

	template <class T> using Ref = std::shared_ptr<T>;
	template <class T, class... Args> Ref<T> make_ref(Args&&... args) { return Ref<T> { new T { std::forward<Args>(args)... } }; }

	template <class T> using Scope = std::unique_ptr<T>;
	template <class T, class... Args> Scope<T> make_scope(Args&&... args) { return Scope<T> { new T { std::forward<Args>(args)... } }; }

	template <class T>
	concept ScopeOrRef = std::is_same_v<T, Scope<T>> || std::is_same_v<T, Ref<T>>;

	template <class To, class From> Ref<To> cast_to(Ref<From> ptr) { return std::dynamic_pointer_cast<To>(ptr); }
	template <class To, class From> decltype(auto) supply_cast(Ref<From> ptr) { return std::dynamic_pointer_cast<To>(ptr)->get(); }

} // namespace Disarray
