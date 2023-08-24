#pragma once

#include <glm/glm.hpp>

namespace Disarray::Maths {

glm::vec3 compute_normal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) { return glm::cross(v2 - v1, v3 - v1); }

} // namespace Disarray::Maths
