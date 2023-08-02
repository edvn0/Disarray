#include "DisarrayPCH.hpp"

#include "core/ReferenceCounted.hpp"

#include "core/Ensure.hpp"

#include <mutex>
#include <unordered_set>

namespace Disarray::MemoryTracking {

	static auto& get_reference_set()
	{
		static std::unordered_set<void*> reference_set {};
		return reference_set;
	}

	static auto& get_reference_mutex()
	{
		static std::mutex reference_mutex;
		return reference_mutex;
	}

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
} // namespace Disarray::MemoryTracking
