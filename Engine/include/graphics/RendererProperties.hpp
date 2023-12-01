#pragma once

#include "Forward.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <array>
#include <cstddef>
#include <optional>

#include "core/Types.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

using ColourVector = glm::vec4;
using TransformMatrix = glm::mat4;

using DescriptorSet = TypeSafeWrapper<std::uint32_t>;
using DescriptorBinding = TypeSafeWrapper<std::uint16_t>;

enum class UBOIdentifier : std::uint8_t {
	Missing,
	ViewProjection,
	Camera,
	PointLight,
	ShadowPass,
	DirectionalLight,
	Glyph,
	SpotLight,
	ImageIndices,
};
template <class T> inline constexpr UBOIdentifier identifier_for = UBOIdentifier::Missing;

enum class RenderPasses : std::uint8_t {
	Text,
	PlanarGeometry,
};

struct RenderAreaExtent {
	Extent offset {};
	Extent extent {};

	explicit RenderAreaExtent(const Disarray::Framebuffer&);
	RenderAreaExtent(const Extent& offset, const Extent& extent);
};

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
	glm::mat4 object_transform;
	glm::vec3 albedo_colour;
	float metalness;
	float roughness;
	float emission;

	float env_map_rotation;

	bool use_normal_map;

	void reset_impl();
};

struct PointLight {
	glm::vec4 position;
	glm::vec4 factors { 1, 0.09F, 0.032F, 0 };
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};
template <> inline constexpr UBOIdentifier identifier_for<PointLight> = UBOIdentifier::PointLight;

struct SpotLight {
	glm::vec4 position;
	glm::vec4 direction_and_cutoff;
	glm::vec4 factors_and_outer_cutoff { 1, 0.09F, 0.032F, glm::cos(glm::radians(25.0F)) };
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};
template <> inline constexpr UBOIdentifier identifier_for<SpotLight> = UBOIdentifier::SpotLight;

namespace Detail {
	template <std::size_t N> struct PointLights : Resettable<PointLights<N>> {
		std::array<PointLight, N> lights {};
		glm::uvec4 count_and_padding {};
		void reset_impl() { lights.fill(PointLight {}); }
	};

	template <std::size_t N> struct SpotLights : Resettable<SpotLights<N>> {
		std::array<SpotLight, N> lights {};
		glm::uvec4 count_and_padding {};
		void reset_impl() { lights.fill(SpotLight {}); }
	};
} // namespace Detail

static constexpr auto max_point_lights = 800;
static constexpr auto count_point_lights = 800;
static_assert(count_point_lights <= max_point_lights);
using PointLights = Detail::PointLights<max_point_lights>;
template <> inline constexpr UBOIdentifier identifier_for<PointLights> = UBOIdentifier::PointLight;

static constexpr auto max_spot_lights = 650;
static constexpr auto count_spot_lights = 650;
static_assert(count_spot_lights <= max_spot_lights);
using SpotLights = Detail::SpotLights<max_spot_lights>;
template <> inline constexpr UBOIdentifier identifier_for<SpotLights> = UBOIdentifier::SpotLight;

struct ViewProjectionUBO : Resettable<ViewProjectionUBO> {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_projection;

	void reset_impl();
};
template <> inline constexpr UBOIdentifier identifier_for<ViewProjectionUBO> = UBOIdentifier::ViewProjection;

struct CameraUBO : Resettable<CameraUBO> {
	glm::vec4 position { 0 };
	glm::vec4 direction { 0 };
	glm::mat4 view { 0 };
	void reset_impl();
};
template <> inline constexpr UBOIdentifier identifier_for<CameraUBO> = UBOIdentifier::Camera;

static constexpr auto default_alignment = 16ULL;
static constexpr auto max_allowed_texture_indices = 46ULL;

struct ImageIndicesUBO : Resettable<ImageIndicesUBO> {
	alignas(default_alignment) std::uint32_t bound_textures { 0 };
	alignas(default_alignment) std::array<glm::uvec4, max_allowed_texture_indices> image_indices { glm::uvec4 { 0, 0, 0, 0 } };

	void reset_impl();
};
template <> inline constexpr UBOIdentifier identifier_for<ImageIndicesUBO> = UBOIdentifier::ImageIndices;

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
template <> inline constexpr UBOIdentifier identifier_for<ShadowPassUBO> = UBOIdentifier::ShadowPass;

struct DirectionalLightUBO : Resettable<DirectionalLightUBO> {
	glm::vec4 position { 0 };
	glm::vec4 direction { 0 };
	glm::vec4 ambient { 0 };
	glm::vec4 diffuse { 0 };
	glm::vec4 specular { 0 };
	glm::vec4 near_far { 0 };

	void reset_impl();
};
template <> inline constexpr UBOIdentifier identifier_for<DirectionalLightUBO> = UBOIdentifier::DirectionalLight;

struct GlyphUBO : Resettable<GlyphUBO> {
	glm::mat4 projection {}; // 64
	glm::mat4 view {}; // 64

	void reset_impl();
};
template <> inline constexpr UBOIdentifier identifier_for<GlyphUBO> = UBOIdentifier::Glyph;

} // namespace Disarray
