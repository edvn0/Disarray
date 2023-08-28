#pragma once

#include "Forward.hpp"

namespace Disarray {

class CppScript {
public:
	virtual ~CppScript() = default;
	virtual void on_create() {};
	virtual void on_update(float) {};
	virtual void on_render(Renderer&) {};
	virtual void on_destroy() {};

	auto identifier() const -> std::string_view { return script_name(); }

protected:
	explicit CppScript(Entity& entity)
		: current_entity(entity) {};

	auto transform() -> Components::Transform&;
	auto transform() const -> const Components::Transform&;

	[[nodiscard]] auto get_entity() -> Entity& { return current_entity; }
	[[nodiscard]] auto get_entity() const -> const Entity& { return current_entity; }
	[[nodiscard]] auto get_scene() -> Scene&;
	[[nodiscard]] auto get_scene() const -> const Scene&;

private:
	virtual auto script_name() const -> std::string_view = 0;

	Entity& current_entity;
};

} // namespace Disarray
