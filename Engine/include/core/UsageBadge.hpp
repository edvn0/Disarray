#pragma once

namespace Disarray {

template <class T> class UsageBadge {
private:
	friend T;
	constexpr UsageBadge() {};
};

} // namespace Disarray
