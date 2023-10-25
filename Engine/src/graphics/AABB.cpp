#include "graphics/AABB.hpp"

namespace Disarray {

auto AABB::calculate_scale_matrix() const -> glm::mat4
{
	glm::vec3 max {
		min_max_x.max,
		min_max_y.max,
		min_max_z.max,
	};
	glm::vec3 min {
		min_max_x.min,
		min_max_y.min,
		min_max_z.min,
	};

	return glm::scale(glm::mat4 { 1.0F }, (max - min));
}

auto AABB::middle_point() const -> glm::vec3
{
	glm::vec3 max {
		min_max_x.max,
		min_max_y.max,
		min_max_z.max,
	};
	glm::vec3 min {
		min_max_x.min,
		min_max_y.min,
		min_max_z.min,
	};

	return (max + min) * 0.5F;
}

} // namespace Disarray
