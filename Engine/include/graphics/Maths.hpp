#pragma once

#include "graphics/GLM.hpp"

namespace Disarray::Maths {

using Length = int;

template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 3)
inline constexpr auto xyz(const glm::vec<L, T, Q>& vec)
{
	return glm::vec3 { vec.x, vec.y, vec.z };
}

template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 2)
inline constexpr auto xy(const glm::vec<L, T, Q>& vec)
{
	return glm::vec2 { vec.x, vec.y };
}

template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L == 4)
inline constexpr auto w(const glm::vec<L, T, Q>& vec)
{
	return vec.w;
}

template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 1 && L <= 4)
inline constexpr auto x(const glm::vec<L, T, Q>& vec)
{
	return vec[0];
}
template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 2 && L <= 4)
inline constexpr auto y(const glm::vec<L, T, Q>& vec)
{
	return vec[1];
}
template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 3 && L <= 4)
inline constexpr auto z(const glm::vec<L, T, Q>& vec)
{
	return vec[2];
}

auto compute_normal(const glm::vec3& first_vertex, const glm::vec3& second_vertex, const glm::vec3& third_vertex) -> glm::vec3;
auto rotate_by(const glm::vec3& axis_radians) -> glm::mat4;
auto ortho(float left_plane, float right_plane, float bottom_plane, float top_plane, float near_plane, float far_plane) -> glm::mat4;

constexpr auto scale_colour(const glm::vec4& maybe_8_bits)
{
	return glm::dot(maybe_8_bits, maybe_8_bits) >= 4.0F ? maybe_8_bits / 255.0F : maybe_8_bits;
}

} // namespace Disarray::Maths
