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

	template <class From, class To> Scope<To> cast(Scope<From> ptr) { return std::static_pointer_cast(ptr); }

} // namespace Disarray
