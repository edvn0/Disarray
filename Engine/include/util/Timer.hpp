#pragma once

#include "core/Clock.hpp"
#include "core/Concepts.hpp"

namespace Disarray {

enum class Granularity { Seconds, Millis, Nanos };
template <Granularity T> constexpr double convert_to_factor = 0;
template <> constexpr double convert_to_factor<Granularity::Seconds> = 1e6;
template <> constexpr double convert_to_factor<Granularity::Millis> = 1e3;
template <> constexpr double convert_to_factor<Granularity::Nanos> = 1;

template <std::floating_point T> class Timer {
public:
	explicit Timer()
		: start_nanos(nanos())
	{
	}
	~Timer() = default;

	template <Granularity Other> T elapsed()
	{
		const double current_nanos = nanos();
		constexpr auto factor = convert_to_factor<Other>;
		const auto diff = current_nanos - start_nanos;
		return diff / factor;
	}

private:
	double nanos() const { return static_cast<double>(Clock::ns()); };

	const double start_nanos { 0.0 };
};

using MSTimer = Timer<float>;

} // namespace Disarray
