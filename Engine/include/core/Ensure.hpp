#pragma once

#include "core/Log.hpp"
#include "core/Types.hpp"

namespace Disarray {

	static constexpr auto ensure(bool to_ensure_is_truthy, const std::string& message = "An error occurred.") -> void
	{
		// To be called like ensure(some_ptr != nullptr, "some message")
#ifdef IS_DEBUG
		if (!to_ensure_is_truthy) {
			Log::error("Assert", message);
			unreachable();
		}
#endif
	}

} // namespace Disarray
