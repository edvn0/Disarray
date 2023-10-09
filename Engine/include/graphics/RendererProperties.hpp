#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <array>
#include <cstddef>
#include <optional>

namespace Disarray {

struct Extent;

enum class Geometry : std::uint8_t {
	Circle,
	Triangle,
	Rectangle,
	Line,
};

struct GeometryProperties {
	glm::vec3 position { 0.0F };
	glm::vec3 scale { 1.0 };
	glm::vec3 to_position { 0.0F };
	glm::vec4 colour { 1.0F };
	glm::quat rotation { glm::identity<glm::quat>() };
	glm::vec3 dimensions { 1.0F };
	std::optional<std::uint32_t> identifier { std::nullopt };
	std::optional<float> radius { std::nullopt };

	template <Geometry T> auto valid() -> bool
	{
		if constexpr (T == Geometry::Circle) {
			return radius.has_value();
		}
		if constexpr (T == Geometry::Triangle || T == Geometry::Rectangle) {
			return !radius.has_value();
		}
		return false;
	}

	[[nodiscard]] auto to_transform() const
	{
		return glm::translate(glm::mat4 { 1.0F }, position) * glm::scale(glm::mat4 { 1.0F }, scale) * glm::mat4_cast(rotation);
	}
};

template <class Child> struct Resettable {
	void reset() { return static_cast<Child&>(*this).reset_impl(); }
};

struct PushConstant : Resettable<PushConstant> {
	static constexpr auto max_image_indices = 8;

	glm::mat4 object_transform { 1.0F };
	glm::vec4 colour { 1.0F };
	std::uint32_t max_identifiers {};
	std::uint32_t current_identifier {};
	std::uint32_t max_point_lights {};
	std::uint32_t bound_textures { 0 };
	std::array<std::int32_t, max_image_indices> image_indices { -1 };

	void reset_impl();
};

struct PointLight {
	glm::vec4 position;
	glm::vec4 factors { 1, 1, 0, 0 };
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

namespace Detail {
	template <std::size_t N> struct PointLights : Resettable<PointLights<N>> {
		std::array<PointLight, N> lights {};
		void reset_impl() { lights.fill(PointLight {}); }
	};
} // namespace Detail

static constexpr auto max_point_lights = 30;
static constexpr auto count_point_lights = 5;
static constexpr auto point_light_radius = 3;
using PointLights = Detail::PointLights<max_point_lights>;

struct UBO : Resettable<UBO> {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_projection;

	void reset_impl();
};

struct CameraUBO : Resettable<CameraUBO> {
	glm::vec4 position { 0 };
	glm::vec4 direction { 0 };
	void reset_impl();
};

static constexpr auto default_alignment = 16ULL;
static constexpr auto max_allowed_texture_indices = 46ULL;

struct ImageIndicesUBO : Resettable<ImageIndicesUBO> {
	alignas(default_alignment) std::uint32_t bound_textures { 0 };
	alignas(default_alignment) std::array<glm::uvec4, max_allowed_texture_indices> image_indices { glm::uvec4 { 0, 0, 0, 0 } };

	void reset_impl();
};

struct ShadowPassUBO : Resettable<ShadowPassUBO> {
	glm::mat4 view {};
	glm::mat4 projection {};
	glm::mat4 view_projection {};

	void reset_impl()
	{
		view = {};
		projection = {};
		view_projection = {};
	}
};

struct DirectionalLightUBO : Resettable<DirectionalLightUBO> {
	glm::vec4 position { 0 };
	glm::vec4 direction { 0 };
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	float near { 0.F };
	float far { 0.F };

	void reset_impl()
	{
		position = {};
		direction = {};
		ambient = {};
		diffuse = {};
		specular = {};
		near = 0;
		far = 0;
	}
};

struct GlyphUBO : Resettable<GlyphUBO> {
	glm::mat4 projection {};

	void reset_impl() { projection = {}; }
};

} // namespace Disarray
