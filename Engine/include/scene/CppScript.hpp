#pragma once

#include <entt/entt.hpp>

#include <utility>
#include <variant>

#include "Forward.hpp"
#include "core/Collections.hpp"
#include "core/PointerDefinition.hpp"

namespace Disarray {

// KV-Pair with general parameters for a script: "Velocity, glm::vec3 {0,-1,0}" e.g.
using Parameter
	= std::variant<std::monostate, glm::vec2, glm::vec3, glm::vec4, float, double, int, std::uint8_t, std::uint32_t, std::size_t, std::string>;

class CppScript {
public:
	virtual ~CppScript() = default;
	virtual void on_create() { }
	virtual void on_update(float /*unused*/) { }
	virtual void on_render(Renderer& /*unused*/) { }
	virtual void on_interface() { }
	virtual void on_destroy() { }

	[[nodiscard]] auto identifier() const -> std::string_view { return script_name(); }

	void update_entity(Scene*, entt::entity);
	auto get_parameters() -> Collections::StringViewMap<Parameter>& { return parameters; }

protected:
	explicit CppScript(const Collections::StringViewMap<Parameter>& params);

	auto transform() -> Components::Transform&;
	[[nodiscard]] auto transform() const -> const Components::Transform&;

	[[nodiscard]] auto get_entity() -> Entity& { return *current_entity; }
	[[nodiscard]] auto get_entity() const -> const Entity& { return *current_entity; }
	[[nodiscard]] auto get_scene() -> Scene&;
	[[nodiscard]] auto get_scene() const -> const Scene&;

private:
	[[nodiscard]] virtual auto script_name() const -> std::string_view = 0;

	Scope<Entity, PimplDeleter<Entity>> current_entity;
	Collections::StringViewMap<Parameter> parameters {};
};

} // namespace Disarray
