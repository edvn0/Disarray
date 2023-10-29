#pragma once

#include <glm/glm.hpp>

#include <magic_enum.hpp>

#include <concepts>
#include <random>
#include <type_traits>

#include "core/KeyCode.hpp"
#include "glm/detail/qualifier.hpp"
#include "glm/geometric.hpp"
#include "glm/gtx/norm.hpp"

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

	static inline std::random_device random_device {};
	static inline std::mt19937 mersenne_twister(random_device());

	template <glm::length_t T, Distribution D> auto random_vector_with_size(float min = 0, float max = 1) -> glm::vec<T, float, glm::packed_highp>
	{
		using distribution = std::conditional_t<D == Distribution::Uniform, std::uniform_real_distribution<float>, std::normal_distribution<float>>;
		distribution dist(min, max);

		GLM_CONSTRUCTOR_ONE_TO_FOUR(dist(mersenne_twister), float);
	}

	template <glm::length_t T, Distribution D>
	auto random_double_vector_with_size(double min = 0, double max = 1) -> glm::vec<T, double, glm::packed_highp>
	{
		using distribution = std::conditional_t<D == Distribution::Uniform, std::uniform_real_distribution<float>, std::normal_distribution<float>>;
		distribution dist(min, max);

		GLM_CONSTRUCTOR_ONE_TO_FOUR(dist(mersenne_twister), double);
	}

	template <std::floating_point Floating = float, Distribution D = Distribution::Uniform>
	auto random_floating_point(Floating min_or_mean = 0.0, Floating max_or_stdev = 1.0) -> Floating
	{
		using distribution
			= std::conditional_t<D == Distribution::Uniform, std::uniform_real_distribution<Floating>, std::normal_distribution<Floating>>;
		distribution dist(min_or_mean, max_or_stdev);

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

template <std::floating_point Floating = float>
auto in_sphere(std::floating_point auto radius, const glm::vec3& center) -> glm::vec<3, Floating, glm::defaultp>
{
	auto position = Detail::random_vector_with_size<3, Distribution::Uniform>(-radius, radius);
	const auto square = radius * radius;
	while (glm::dot(position, position) > square) {
		position = Detail::random_vector_with_size<3, Distribution::Uniform>();
	}

	return position - center;
}

template <std::floating_point Floating = float> auto in_sphere(std::floating_point auto radius = 1.0F) -> glm::vec<3, Floating, glm::defaultp>
{
	return in_sphere(radius, { 0.0, 0.0, 0.0 });
}

template <std::floating_point Floating = float> auto on_sphere(std::floating_point auto radius = 1.0F) -> glm::vec<3, Floating, glm::defaultp>
{
	const auto vector = glm::normalize(Detail::random_vector_with_size<3, Distribution::Normal>());
	return vector * radius;
}

template <Distribution D = Distribution::Uniform> auto colour() -> glm::vec<4, float, glm::packed_highp>
{
	glm::vec<4, float, glm::packed_highp> colour = random<4, float, D>(0.F, 1.F);
	colour[3] = 1.0F;
	return colour;
}

template <std::size_t Count> auto choose_from(auto& engine)
{
	std::uniform_int_distribution dist(0, static_cast<int>(Count - 1));
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

template <typename Enum>
	requires(std::is_enum_v<Enum>)
auto as_enum(Enum default_enum = {})
{
	const auto random_int = Random::choose_from<magic_enum::enum_count<Enum>()>(Detail::mersenne_twister);
	return magic_enum::enum_cast<Enum>(random_int).value_or(default_enum);
}

} // namespace Disarray::Random
