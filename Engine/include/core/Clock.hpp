#pragma once

namespace Disarray {

class Clock {
public:
	static auto ms() -> double;
	static auto ns() -> double;

	template <std::floating_point T = float> static auto nanos() { return static_cast<T>(ns()); }
	template <std::floating_point T = float> static auto millis() { return static_cast<T>(ms()); }
};

} // namespace Disarray
