#include "DisarrayPCH.hpp"

#include "core/ReferenceCounted.hpp"

#include <mutex>
#include <unordered_set>

#include "core/Ensure.hpp"

namespace Disarray {

ReferenceCountable::~ReferenceCountable() = default;

void ReferenceCountable::increment_reference_count() { ++reference_count; }

void ReferenceCountable::decrement_reference_count() { --reference_count; }

auto ReferenceCountable::get_reference_count() const -> std::uint32_t { return reference_count; }

namespace MemoryTracking {
	namespace {

		auto get_reference_set() -> auto&
		{
			static std::unordered_set<void*> reference_set {};
			return reference_set;
		}

		auto get_reference_mutex() -> auto&
		{
			static std::mutex reference_mutex;
			return reference_mutex;
		}

	} // namespace

	void register_dead_impl(void* instance)
	{
		std::scoped_lock lock(get_reference_mutex());
		ensure(instance != nullptr, "Instance was for some reason already nullptr.");
		ensure(get_reference_set().contains(instance), "Instance was not registered.");
		get_reference_set().erase(instance);
	}

	void register_alive_impl(void* instance)
	{
		std::scoped_lock lock(get_reference_mutex());
		auto& reference_set = get_reference_set();
		ensure(instance != nullptr, "Instance was for some reason already nullptr.");
		reference_set.insert(instance);
	}
} // namespace MemoryTracking

} // namespace Disarray
