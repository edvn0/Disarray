#pragma once

#include <functional>
#include <string_view>

namespace Disarray {

	struct StringHash {
		using is_transparent = void;

		std::size_t operator()(std::string_view sv) const
		{
			std::hash<std::string_view> hasher;
			return hasher(sv);
		}
	};

} // namespace Disarray
