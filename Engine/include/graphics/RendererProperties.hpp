#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cstddef>
#include <optional>

namespace Disarray {

struct Extent;

enum class Geometry {
	Circle,
	Triangle,
	Rectangle,
	Line,
};

struct GeometryProperties {
	glm::vec3 position { 0.0f };
	glm::vec3 scale { 1.0 };
	glm::vec3 to_position { 0.0f };
	glm::vec4 colour { 1.0f };
	glm::quat rotation { glm::identity<glm::quat>() };
	glm::vec3 dimensions { 1.f };
	std::optional<std::uint32_t> identifier { std::nullopt };
	std::optional<float> radius { std::nullopt };

	template <Geometry T> bool valid()
	{
		if constexpr (T == Geometry::Circle) {
			return radius.has_value();
		}
		if constexpr (T == Geometry::Triangle || T == Geometry::Rectangle) {
			return !radius.has_value();
		}
		return false;
	}

	auto to_transform() const
	{
		return glm::translate(glm::mat4 { 1.0f }, position) * glm::scale(glm::mat4 { 1.0f }, scale) * glm::mat4_cast(rotation);
	}
};

struct PushConstant {
	glm::mat4 object_transform { 1.0f };
	glm::vec4 colour { 1.0f };
	std::uint32_t max_identifiers {};
	std::uint32_t current_identifier {};
	std::uint32_t max_point_lights {};
};

struct PointLight {
	glm::vec4 position;
	glm::vec4 factors { 1, 1, 0, 0 };
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

using PointLights = std::array<PointLight, 30>;

struct UBO {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_projection;
	glm::vec4 sun_direction_and_intensity { 1.0 };
	glm::vec4 sun_colour { 1.0f };
};

struct CameraUBO {
	glm::vec4 position { 0 };
	glm::vec4 direction { 0 };
};

} // namespace Disarray
