#pragma once

#include <cstdint>

#include "core/Clock.hpp"

namespace Disarray {

enum class Granularity : std::uint8_t { Seconds, Millis, Nanos };
template <Granularity T> inline constexpr double convert_from_nano_seconds_to_factor = 0;
template <> inline constexpr double convert_from_nano_seconds_to_factor<Granularity::Seconds> = 1e6;
template <> inline constexpr double convert_from_nano_seconds_to_factor<Granularity::Millis> = 1e3;
template <> inline constexpr double convert_from_nano_seconds_to_factor<Granularity::Nanos> = 1;

template <std::floating_point T> class Timer {
public:
	explicit Timer()
		: start_nanos(nanos())
	{
	}
	~Timer() = default;

	template <Granularity Other> auto elapsed() -> T
	{
		const double current_nanos = nanos();
		constexpr auto factor = convert_from_nano_seconds_to_factor<Other>;
		const auto diff = current_nanos - start_nanos;
		return diff / factor;
	}

private:
	[[nodiscard]] auto nanos() const -> double { return static_cast<double>(Clock::ns()); };

	double start_nanos { 0.0 };
};

using MSTimer = Timer<float>;

} // namespace Disarray
