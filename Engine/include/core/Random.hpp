#pragma once

#include <glm/glm.hpp>

#include <concepts>
#include <random>

#define GLM_CONSTRUCTOR_ONE_TO_FOUR(x, FloatType)                                                                                                    \
	if constexpr (T == 1) {                                                                                                                          \
		return glm::vec<T, FloatType, glm::packed_highp> { (x) };                                                                                    \
	}                                                                                                                                                \
	if constexpr (T == 2) {                                                                                                                          \
		return glm::vec<T, FloatType, glm::packed_highp> { (x), (x) };                                                                               \
	}                                                                                                                                                \
	if constexpr (T == 3) {                                                                                                                          \
		return glm::vec<T, FloatType, glm::packed_highp> { (x), (x), (x) };                                                                          \
	}                                                                                                                                                \
	if constexpr (T == 4) {                                                                                                                          \
		return glm::vec<T, FloatType, glm::packed_highp> { (x), (x), (x), (x) };                                                                     \
	}

namespace Disarray::Random {

enum class Distribution : std::uint8_t { Normal, Uniform };

namespace Detail {

	static inline std::random_device random_device;
	static inline std::mt19937 mersenne_twister(random_device());

	template <glm::length_t T, Distribution D> auto random_vector_with_size(float min = 0, float max = 1) -> glm::vec<T, float, glm::packed_highp>
	{
		std::uniform_real_distribution<float> dist(min, max);

		GLM_CONSTRUCTOR_ONE_TO_FOUR(dist(mersenne_twister), float);
	}

	template <glm::length_t T, Distribution D>
	auto random_double_vector_with_size(double min = 0, double max = 1) -> glm::vec<T, double, glm::packed_highp>
	{
		std::uniform_real_distribution<double> dist(min, max);

		GLM_CONSTRUCTOR_ONE_TO_FOUR(dist(mersenne_twister), double);
	}

	template <std::floating_point Floating = float, Distribution D = Distribution::Uniform>
	auto random_floating_point(Floating min, Floating max) -> Floating
	{
		std::uniform_real_distribution<Floating> dist(min, max);
		return dist(mersenne_twister);
	}
} // namespace Detail

template <glm::length_t T = 1, std::floating_point Floating = float, Distribution D = Distribution::Uniform>
auto random(std::floating_point auto min = 0.0, std::floating_point auto max = 1.0) -> decltype(auto)
{
	if constexpr (std::is_same_v<Floating, float>) {
		return Detail::random_vector_with_size<T, D>(min, max);
	}

	if constexpr (std::is_same_v<Floating, double>) {
		return Detail::random_double_vector_with_size<T, D>(min, max);
	}

	if constexpr (std::is_same_v<Floating, long double>) {
		return Detail::random_double_vector_with_size<T, D>(min, max);
	}
}
template <std::floating_point Floating = float, Distribution D = Distribution::Uniform>
auto as_double(std::floating_point auto min = 0.0, std::floating_point auto max = 1.0) -> Floating
{
	return Detail::random_floating_point<Floating, D>(min, max);
}

template <Distribution D = Distribution::Uniform> auto colour() -> glm::vec<4, float, glm::packed_highp>
{
	glm::vec<4, float, glm::packed_highp> colour = random<4, float, D>(0.F, 1.F);
	colour[3] = 1.0F;
	return colour;
}

template <std::size_t Count> static auto choose_from(auto& engine)
{
	std::uniform_int_distribution dist(0, static_cast<int>(Count));
	return dist(engine);
};

template <Distribution D = Distribution::Uniform> auto strong_colour() -> glm::vec<4, float, glm::packed_highp>
{
	glm::vec<4, float, glm::packed_highp> colour = random<4, float, D>(0.0F, 1.F);
	colour.a = 1.0F;

	auto index = choose_from<4>(Detail::mersenne_twister);
	colour[index] = 1.0F;
	return colour;
}

} // namespace Disarray::Random
