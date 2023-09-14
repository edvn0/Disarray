#pragma once

#include <glm/glm.hpp>

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
inline constexpr auto xy(const glm::vec<L, T>& vec)
{
	return glm::vec2 { vec.x, vec.y };
}

template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L == 4)
inline constexpr auto w(const glm::vec<L, T>& vec)
{
	return vec.w;
}

template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 1 && L <= 4)
inline constexpr auto x(const glm::vec<L, T>& vec)
{
	return vec[0];
}
template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 2 && L <= 4)
inline constexpr auto y(const glm::vec<L, T>& vec)
{
	return vec[1];
}
template <Length L, class T, glm::qualifier Q = glm::defaultp>
	requires(L >= 3 && L <= 4)
inline constexpr auto z(const glm::vec<L, T>& vec)
{
	return vec[2];
}

inline auto compute_normal(const glm::vec3& first_vertex, const glm::vec3& second_vertex, const glm::vec3& third_vertex) -> glm::vec3
{
	return glm::cross(second_vertex - first_vertex, third_vertex - first_vertex);
}

inline auto rotate_by(const glm::vec3& axis_radians)
{
  glm::mat4 identity {1.0F};
	identity = glm::rotate(identity, x(axis_radians), glm::vec3 { 1.F, 0, 0 });
	identity = glm::rotate(identity, y(axis_radians), glm::vec3 { 0, 1.F, 0 });
	identity = glm::rotate(identity, z(axis_radians), glm::vec3 { 0, 0, 1.F });
  return identity;
}

} // namespace Disarray::Maths
