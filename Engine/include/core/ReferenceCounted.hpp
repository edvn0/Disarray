#pragma once

#include "util/BitCast.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace Disarray {

class ReferenceCountable {
public:
	virtual ~ReferenceCountable() = default;

	void increment_reference_count() { ++reference_count; }

	void decrement_reference_count() { --reference_count; }

	std::uint32_t get_reference_count() const { return reference_count; }

private:
	std::atomic<uint32_t> reference_count = 0;
};

namespace MemoryTracking {
	void register_dead_impl(void*);

	void register_alive_impl(void*);

	template <class T> void register_dead(T* instance) { register_dead_impl(bit_cast<void*>(instance)); }

	template <class T> void register_alive(T* instance) { register_alive_impl(bit_cast<void*>(instance)); }
} // namespace MemoryTracking

template <class T> class ReferenceCounted {
public:
	ReferenceCounted()
		: instance(nullptr)
	{
		static_assert(std::is_base_of_v<ReferenceCountable, T>, "T must inherit from ReferenceCountable");
	}

	ReferenceCounted(std::nullptr_t)
		: instance(nullptr)
	{
		static_assert(std::is_base_of_v<ReferenceCountable, T>, "T must inherit from ReferenceCountable");
	}

	ReferenceCounted(T* inst)
		: instance(inst)
	{
		static_assert(std::is_base_of_v<ReferenceCountable, T>, "T must inherit from ReferenceCountable");
		increment_reference_count();
	}

	template <typename... Args, std::enable_if_t<sizeof...(Args) == 0, bool> = false>
	ReferenceCounted(Args&&... args)
		requires(std::is_constructible_v<T, Args...>)
		: instance(new T { std::forward<Args>(args)... })
	{
		static_assert(std::is_base_of_v<ReferenceCountable, T>, "T must inherit from ReferenceCountable");
		increment_reference_count();
	}

	ReferenceCounted(ReferenceCounted<T>&& other)
		: instance(std::move(other.instance))
	{
		increment_reference_count();
	}

	ReferenceCounted(const ReferenceCounted<T>& other)
		: instance(other.instance)
	{
		increment_reference_count();
	}

	~ReferenceCounted() { decrement_reference_count(); }

	template <class T2> ReferenceCounted(const ReferenceCounted<T2>& other)
	{
		instance = static_cast<T*>(other.instance);
		increment_reference_count();
	}

	template <class T2> ReferenceCounted(ReferenceCounted<T2>&& other)
	{
		auto* other_instance = std::move(other.instance);
		instance = static_cast<T*>(other_instance);
		increment_reference_count();
	}

	static ReferenceCounted<T> copy_without_increment(const ReferenceCounted<T>& other)
	{
		ReferenceCounted<T> result = nullptr;
		result->instance = other.instance;
		return result;
	}

	ReferenceCounted& operator=(std::nullptr_t)
	{
		decrement_reference_count();
		instance = nullptr;
		return *this;
	}

	ReferenceCounted& operator=(const ReferenceCounted<T>& other)
	{
		other.increment_reference_count();
		decrement_reference_count();

		instance = other.instance;
		return *this;
	}

	template <class T2> ReferenceCounted& operator=(const ReferenceCounted<T2>& other)
	{
		other.increment_reference_count();
		decrement_reference_count();

		instance = other.instance;
		return *this;
	}

	template <class T2> ReferenceCounted& operator=(ReferenceCounted<T2>&& other)
	{
		decrement_reference_count();

		instance = other.instance;
		other.instance = nullptr;
		return *this;
	}

	operator bool() { return instance != nullptr; }

	operator bool() const { return instance != nullptr; }

	T* operator->() { return instance; }

	const T* operator->() const { return instance; }

	T& operator*() { return *instance; }

	const T& operator*() const { return *instance; }

	T* get() { return instance; }

	const T* get() const { return instance; }

	void reset(T* inst = nullptr)
	{
		decrement_reference_count();
		instance = inst;
	}

	template <class T2> ReferenceCounted<T2> as() const { return ReferenceCounted<T2>(*this); }

	template <typename... Args> static ReferenceCounted<T> construct(Args&&... args)
	{
		return ReferenceCounted<T>(new T(std::forward<Args>(args)...));
	}

	bool operator==(const ReferenceCounted<T>& other) const { return instance == other.instance; }

	bool operator!=(const ReferenceCounted<T>& other) const { return !(*this == other); }

	bool equals(const ReferenceCounted<T>& other) const
	{
		if (!instance || !other.instance)
			return false;

		return *instance == *other.instance;
	}

private:
	void increment_reference_count() const
	{
		if (instance) {
			instance->increment_reference_count();
			MemoryTracking::register_alive(instance);
		}
	}

	void decrement_reference_count() const
	{
		if (instance) {
			instance->decrement_reference_count();
			if (instance->get_reference_count() == 0) {
				delete instance;
				MemoryTracking::register_dead(instance);
				instance = nullptr;
			}
		}
	}

	template <class T2> friend class ReferenceCounted;

	mutable T* instance;
};

} // namespace Disarray
