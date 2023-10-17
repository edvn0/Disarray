#pragma once

#include <entt/entt.hpp>

#include <concepts>
#include <utility>
#include <variant>

#include "Forward.hpp"
#include "core/Collections.hpp"
#include "core/Concepts.hpp"
#include "core/PointerDefinition.hpp"

#define DISARRAY_FOR_TYPE(Type)                                                                                                                      \
	if constexpr (std::is_same_v<T, Type>) {                                                                                                         \
		if (std::holds_alternative<Type>(parameter)) {                                                                                               \
			return func(std::get<Type>(parameter));                                                                                                  \
		}                                                                                                                                            \
		return false;                                                                                                                                \
	}

namespace Disarray {

// KV-Pair with general parameters for a script: "Velocity, glm::vec3 {0,-1,0}" e.g.
using Parameter
	= std::variant<std::monostate, glm::vec2, glm::vec3, glm::vec4, float, double, int, std::uint8_t, std::uint32_t, std::size_t, std::string>;

template <class T>
	requires((AnyOf<T, std::monostate, glm::vec2, glm::vec3, glm::vec4, float, double, int, std::uint8_t, std::uint32_t, std::size_t, std::string>))
static constexpr auto switch_parameter = [](Parameter& parameter, auto&& func) -> bool {
	DISARRAY_FOR_TYPE(std::monostate);
	DISARRAY_FOR_TYPE(glm::vec2);
	DISARRAY_FOR_TYPE(glm::vec3);
	DISARRAY_FOR_TYPE(glm::vec4);
	DISARRAY_FOR_TYPE(float);
	DISARRAY_FOR_TYPE(double);
	DISARRAY_FOR_TYPE(int);
	DISARRAY_FOR_TYPE(std::uint8_t);
	DISARRAY_FOR_TYPE(std::uint32_t);
	DISARRAY_FOR_TYPE(std::size_t);
	DISARRAY_FOR_TYPE(std::string);

	return false;
};

class CppScript {
public:
	virtual ~CppScript() = default;
	virtual void on_create() { }
	virtual void on_update(float /*unused*/) { }
	virtual void on_render(Renderer& /*unused*/) { }
	virtual void on_interface() { }
	virtual void on_destroy() { }
	virtual void reload() = 0;

	[[nodiscard]] auto identifier() const -> std::string_view { return script_name(); }

	void update_entity(Scene*, entt::entity);
	auto get_parameters() -> Collections::StringViewMap<Parameter>& { return parameters; }
	auto get_parameter_or(std::string_view key, const Parameter& default_value = std::monostate {}) -> const Parameter&
	{
		if (parameters.contains(key)) {
			return parameters.at(key);
		}

		return default_value;
	}

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
