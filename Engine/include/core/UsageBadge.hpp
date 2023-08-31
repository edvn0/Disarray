#pragma once

namespace Disarray {

template <class T> class UsageBadge {
private:
	friend T;
	constexpr UsageBadge() {};
};

template <class T> class PolymorphicUsageBadge {
private:
	template <class Other> constexpr PolymorphicUsageBadge()
	{
		static_assert(std::is_base_of_v<T, Other>);
		UsageBadge<Other> badge {};
	};
};

} // namespace Disarray
