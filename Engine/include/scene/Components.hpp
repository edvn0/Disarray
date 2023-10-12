#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <entt/entt.hpp>

#include <future>
#include <memory>
#include <string_view>
#include <unordered_set>

#include "Forward.hpp"
#include "core/Collections.hpp"
#include "core/Concepts.hpp"
#include "core/Log.hpp"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "graphics/Material.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/Texture.hpp"
#include "scene/CppScript.hpp"

namespace Disarray::Components {

template <class T> static inline constexpr std::string_view component_name;

namespace {
	const auto default_rotation = glm::angleAxis(0.0F, glm::vec3 { 0.0F, 0.0F, 1.0F });
	constexpr auto identity = glm::identity<glm::mat4>();
	inline auto scale_matrix(const auto& vec) { return glm::scale(identity, vec); }
	inline auto translate_matrix(const auto& vec) { return glm::translate(identity, vec); }
} // namespace

struct Transform {
	glm::quat rotation { default_rotation };
	glm::vec3 position { 0.0F };
	glm::vec3 scale { 1.0F };

	Transform() = default;
	Transform(const glm::vec3& euler, const glm::vec3& pos, const glm::vec3& scl)
		: rotation(euler)
		, position(pos)
		, scale(scl)
	{
	}

	[[nodiscard]] auto compute() const { return translate_matrix(position) * glm::mat4_cast(rotation) * scale_matrix(scale); }
};
template <> inline constexpr std::string_view component_name<Transform> = "Transform";

struct Mesh {
	Mesh() = default;
	// Deserialisation constructor :)
	explicit Mesh(Device&, std::string_view path);
	explicit Mesh(Ref<Disarray::Mesh>);

	Ref<Disarray::Mesh> mesh { nullptr };
};
template <> inline constexpr std::string_view component_name<Mesh> = "Mesh";

struct Material {
	Material() = default;
	explicit Material(Device&, std::string_view vertex_path, std::string_view fragment_path);
	explicit Material(Ref<Disarray::Material>);
	Ref<Disarray::Material> material { nullptr };
};
template <> inline constexpr std::string_view component_name<Material> = "Material";

struct Pipeline {
	Pipeline() = default;
	explicit Pipeline(Ref<Disarray::Pipeline>);
	Ref<Disarray::Pipeline> pipeline { nullptr };
};
template <> inline constexpr std::string_view component_name<Pipeline> = "Pipeline";

struct Texture {
	Texture() = default;
	explicit Texture(Ref<Disarray::Texture>, const glm::vec4& = glm::vec4 { 1.0F });
	explicit Texture(const glm::vec4&);
	// Deserialisation constructor :)
	explicit Texture(Device&, std::string_view path);
	Ref<Disarray::Texture> texture { nullptr };
	glm::vec4 colour { 1.0F };
};
template <> inline constexpr std::string_view component_name<Texture> = "Texture";

struct LineGeometry {
	explicit LineGeometry(const glm::vec3& pos)
		: to_position(pos)
	{
	}
	LineGeometry() = default;
	glm::vec3 to_position { 0.0F };
	Disarray::Geometry geometry { Disarray::Geometry::Line };
};
template <> inline constexpr std::string_view component_name<LineGeometry> = "LineGeometry";

struct QuadGeometry {
	Disarray::Geometry geometry { Disarray::Geometry::Rectangle };
};
template <> inline constexpr std::string_view component_name<QuadGeometry> = "QuadGeometry";

struct ID {
	Identifier identifier {};

	template <std::integral T> [[nodiscard]] T get_id() const { return static_cast<T>(identifier); }
};
template <> inline constexpr std::string_view component_name<ID> = "ID";

struct Tag {
	std::string name {};
};
template <> inline constexpr std::string_view component_name<Tag> = "Tag";

struct DirectionalLight {
	struct ProjectionParameters {
		float left { -5.F };
		float right { 5.F };
		float bottom { -5.F };
		float top { 5.F };
		float near { 0.1F };
		float far { 50.F };
		float fov { 45.0F };

		[[nodiscard]] auto compute() const -> glm::mat4;
	};
	ProjectionParameters projection_parameters {};
	glm::vec4 position { 0 };
	glm::vec4 direction { 1 };
	glm::vec4 ambient { 1 };
	glm::vec4 diffuse { 1 };
	glm::vec4 specular { 1 };

	bool use_direction_vector { false };

	DirectionalLight() = default;
	DirectionalLight(const glm::vec4& ambience)
		: ambient(ambience)
	{
	}
	DirectionalLight(const glm::vec4& ambience, ProjectionParameters params)
		: projection_parameters(params)
		, ambient(ambience)
	{
	}
};
template <> inline constexpr std::string_view component_name<DirectionalLight> = "DirectionalLight";

struct PointLight {
	glm::vec4 diffuse { 0.F };
	glm::vec4 specular { 0.F };
	glm::vec4 ambient { 0.F };
	glm::vec4 factors { 1, 0.09F, 0.032F, 0 };
};
template <> inline constexpr std::string_view component_name<PointLight> = "PointLight";

struct Script {
	Script() = default;

	template <class ChildScript, typename... Args>
		requires std::is_base_of_v<CppScript, ChildScript>
	void bind(Args&&... args)
	{
		instance_slot = ScriptPtr { new ChildScript { std::forward<Args>(args)... } };
		setup_entity_creation();
		setup_entity_destruction();
		bound = true;
	}

	template <class ChildScript>
		requires std::is_base_of_v<CppScript, ChildScript>
	void bind(const Collections::StringViewMap<Parameter>& script_parameters)
	{
		instance_slot = ScriptPtr { new ChildScript { script_parameters } };
		setup_entity_creation();
		setup_entity_destruction();
		bound = true;
	}

	void destroy();
	void instantiate();
	auto get_script() -> CppScript&;
	[[nodiscard]] auto get_script() const -> const CppScript&;
	[[nodiscard]] auto has_been_bound() const -> bool;

private:
	friend class Disarray::Entity;
	struct Deleter {
		void operator()(CppScript* script) noexcept;
	};
	using ScriptPtr = std::unique_ptr<CppScript, Deleter>;

	void setup_entity_destruction();
	void setup_entity_creation();

	ScriptPtr instance_slot { nullptr };

	std::function<void(Script&)> create_script_functor;
	std::function<void(Script&)> destroy_script_functor;

	bool bound { false };
	bool instantiated { false };
};
template <> inline constexpr std::string_view component_name<Script> = "Script";

struct Inheritance {
	std::unordered_set<Identifier> children {};
	Identifier parent {};

	void add_child(Entity&);

	[[nodiscard]] auto has_parent() const -> bool { return parent != invalid_identifier; }
	[[nodiscard]] auto has_children() const -> bool { return !children.empty(); }
};
template <> inline constexpr std::string_view component_name<Inheritance> = "Inheritance";

} // namespace Disarray::Components
