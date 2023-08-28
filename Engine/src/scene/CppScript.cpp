#include "scene/CppScript.hpp"

#include "scene/Components.hpp"
#include "scene/Entity.hpp"
#include "scene/Scene.hpp"

namespace Disarray {

auto CppScript::get_scene() const -> const Scene& { return *current_entity.scene; }

auto CppScript::get_scene() -> Scene& { return *current_entity.scene; }

auto CppScript::transform() -> Components::Transform& { return get_entity().get_components<Components::Transform>(); }

auto CppScript::transform() const -> const Components::Transform& { return get_entity().get_components<Components::Transform>(); }

} // namespace Disarray
