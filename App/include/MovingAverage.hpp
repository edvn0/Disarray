#pragma once

#include "core/Concepts.hpp"

#include <array>

namespace Disarray::Client {

template <IsNumber T, IsNumber Total, std::size_t N> class MovingAverage {
public:
	MovingAverage() { samples.fill(T {}); }

	MovingAverage& operator()(T sample)
	{
		total += sample;
		if (num_samples < N)
			samples[num_samples++] = sample;
		else {
			T& oldest = samples[num_samples++ % N];
			total -= oldest;
			oldest = sample;
		}
		return *this;
	}

	operator double() const { return total / std::min(num_samples, N); }
	operator float() const { return static_cast<float>(total) / std::min(num_samples, N); }

	double inverse() const { return std::min(num_samples, N) / total; }

private:
	std::array<T, N> samples {};
	std::size_t num_samples { 0 };
	Total total { 0 };
};
} // namespace Disarray::Client
