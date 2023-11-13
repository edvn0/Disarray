#include "graphics/Maths.hpp"

namespace Disarray::Maths {

auto compute_normal(const glm::vec3& first_vertex, const glm::vec3& second_vertex, const glm::vec3& third_vertex) -> glm::vec3
{
	const auto normalized_v1 = glm::normalize(second_vertex - first_vertex);
	const auto normalized_v2 = glm::normalize(third_vertex - first_vertex);
	return glm::cross(normalized_v1, normalized_v2);
}

auto rotate_by(const glm::vec3& axis_radians) -> glm::mat4
{
	glm::mat4 identity { 1.0F };
	identity = glm::rotate(identity, x(axis_radians), glm::vec3 { 1.F, 0, 0 });
	identity = glm::rotate(identity, y(axis_radians), glm::vec3 { 0, 1.F, 0 });
	identity = glm::rotate(identity, z(axis_radians), glm::vec3 { 0, 0, 1.F });
	return identity;
}

auto ortho(float left_plane, float right_plane, float bottom_plane, float top_plane, float near_plane, float far_plane) -> glm::mat4
{
	auto ortho = glm::ortho(left_plane, right_plane, bottom_plane, top_plane, near_plane, far_plane);
	return ortho;
}

} // namespace Disarray::Maths
