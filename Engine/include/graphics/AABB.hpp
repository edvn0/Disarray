#pragma once

#include <glm/glm.hpp>

namespace Disarray {

enum class AABBAxis : std::uint8_t {
	X,
	Y,
	Z,
};

struct AABBRange {
	float min {};
	float max {};
};
class AABB {

public:
	AABB() = default;
	constexpr AABB(const glm::vec2& for_x, const glm::vec2& for_y, const glm::vec2& for_z) noexcept
		: min_max_x(for_x.x, for_x.y)
		, min_max_y(for_y.x, for_y.y)
		, min_max_z(for_z.x, for_z.y)
	{
	}

	constexpr AABB(const AABBRange& for_x, const AABBRange& for_y, const AABBRange& for_z) noexcept
		: min_max_x(for_x)
		, min_max_y(for_y)
		, min_max_z(for_z)
	{
	}

	template <AABBAxis Axis> auto for_axis() const
	{
		if constexpr (AABBAxis::X == Axis) {
			return min_max_x;
		}
		if constexpr (AABBAxis::Y == Axis) {
			return min_max_y;
		}
		if constexpr (AABBAxis::Z == Axis) {
			return min_max_z;
		}
	}

	[[nodiscard]] auto calculate_scale_matrix() const -> glm::mat4;
	[[nodiscard]] auto middle_point() const -> glm::vec3;

private:
	AABBRange min_max_x {};
	AABBRange min_max_y {};
	AABBRange min_max_z {};
};

} // namespace Disarray
