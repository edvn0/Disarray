#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <entt/entt.hpp>

#include <memory>
#include <string_view>
#include <unordered_set>

#include "Forward.hpp"
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

namespace Disarray::Components {

namespace {
	const auto default_rotation = glm::angleAxis(0.f, glm::vec3 { 0.f, 0.f, 1.0f });
	constexpr auto identity = glm::identity<glm::mat4>();
	inline auto scale_matrix(const auto& vec) { return glm::scale(identity, vec); }
	inline auto translate_matrix(const auto& vec) { return glm::translate(identity, vec); }
} // namespace

struct Transform {
	glm::quat rotation { default_rotation };
	glm::vec3 position { 0.0f };
	glm::vec3 scale { 1.0f };

	Transform() = default;
	Transform(const glm::vec3& euler, const glm::vec3& pos, const glm::vec3& scl)
		: rotation(euler)
		, position(pos)
		, scale(scl)
	{
	}

	[[nodiscard]] auto compute() const { return translate_matrix(position) * glm::mat4_cast(rotation) * scale_matrix(scale); }
};

struct Mesh {
	Mesh() = default;
	// Deserialisation constructor :)
	explicit Mesh(Device&, std::string_view path);
	explicit Mesh(Ref<Disarray::Mesh>);
	Ref<Disarray::Mesh> mesh { nullptr };
};

struct Material {
	Material() = default;
	explicit Material(Device&, std::string_view vertex_path, std::string_view fragment_path);
	explicit Material(Ref<Disarray::Material>);
	Ref<Disarray::Material> material { nullptr };
};

struct Pipeline {
	Pipeline() = default;
	explicit Pipeline(Ref<Disarray::Pipeline>);
	Ref<Disarray::Pipeline> pipeline { nullptr };
};

struct Texture {
	Texture() = default;
	explicit Texture(Ref<Disarray::Texture>, const glm::vec4& = glm::vec4 { 1.0f });
	explicit Texture(const glm::vec4&);
	// Deserialisation constructor :)
	explicit Texture(Device&, std::string_view path);
	Ref<Disarray::Texture> texture { nullptr };
	glm::vec4 colour { 1.0f };
};

struct LineGeometry {
	explicit LineGeometry(const glm::vec3& pos)
		: to_position(pos)
	{
	}
	LineGeometry() = default;
	glm::vec3 to_position { 0.0f };
	Disarray::Geometry geometry { Disarray::Geometry::Line };
};

struct QuadGeometry {
	Disarray::Geometry geometry { Disarray::Geometry::Rectangle };
};

struct ID {
	Identifier identifier {};

	template <std::integral T> [[nodiscard]] T get_id() const { return static_cast<T>(identifier); }
};

struct Tag {
	std::string name {};
};

struct DirectionalLight {
	glm::vec3 direction { 1, 1, 1 };
	float intensity { .05f };

	[[nodiscard]] auto compute() const -> glm::vec4 { return { glm::normalize(direction), intensity }; }
};

struct PointLight {
	glm::vec3 direction { 0.f };
	float intensity { .01f };
};

void script_deleter(CppScript*);
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

	void destroy();
	void instantiate();
	auto get_script() -> CppScript&;
	[[nodiscard]] auto get_script() const -> const CppScript&;
	[[nodiscard]] auto has_been_bound() const -> bool;

private:
	friend class Disarray::Entity;
	struct Deleter {
		void operator()(CppScript* ptr) { script_deleter(ptr); }
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

struct Inheritance {
	std::unordered_set<Identifier> children {};
	Identifier parent {};

	void add_child(Entity&);

	[[nodiscard]] auto has_parent() const -> bool { return parent != invalid_identifier; }
};

} // namespace Disarray::Components
