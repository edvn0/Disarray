#pragma once

#include <glm/glm.hpp>

namespace Disarray::Maths {

glm::vec3 compute_normal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) { return glm::cross(v2 - v1, v3 - v1); }

template <std::size_t L, class T>
	requires(L >= 3)
inline auto xyz(const glm::vec<L, T>& vec)
{
	return glm::vec3 { vec.x, vec.y, vec.z };
}

template <std::size_t L, class T>
	requires(L >= 2)
inline auto xy(const glm::vec<L, T>& vec)
{
	return glm::vec2 { vec.x, vec.y };
}

template <std::size_t L, class T>
	requires(L == 4)
inline auto w(const glm::vec<L, T>& vec)
{
	return vec.w;
}

} // namespace Disarray::Maths
