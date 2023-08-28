#pragma once

#include <utility>

#include "Forward.hpp"
#include "scene/Entity.hpp"

namespace Disarray {

class CppScript {
public:
	virtual ~CppScript() = default;
	virtual void on_create() {};
	virtual void on_update(float /*unused*/) {};
	virtual void on_render(Renderer& /*unused*/) {};
	virtual void on_destroy() {};

	[[nodiscard]] auto identifier() const -> std::string_view { return script_name(); }

	void update_entity(Entity entity) { current_entity = std::move(entity); }

protected:
	auto transform() -> Components::Transform&;
	[[nodiscard]] auto transform() const -> const Components::Transform&;

	[[nodiscard]] auto get_entity() -> Entity& { return current_entity; }
	[[nodiscard]] auto get_entity() const -> const Entity& { return current_entity; }
	[[nodiscard]] auto get_scene() -> Scene&;
	[[nodiscard]] auto get_scene() const -> const Scene&;

private:
	[[nodiscard]] virtual auto script_name() const -> std::string_view = 0;

	Entity current_entity;
};

} // namespace Disarray
