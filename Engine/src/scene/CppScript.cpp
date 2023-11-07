#include "scene/CppScript.hpp"

#include "core/PointerDefinition.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"
#include "scene/Scene.hpp"

namespace Disarray {

template <> auto PimplDeleter<Entity>::operator()(Entity* ptr) noexcept -> void { delete ptr; }

void CppScript::update_entity(Scene* scene, entt::entity handle) { *current_entity = Entity { scene, handle }; }

auto CppScript::get_scene() const -> const Scene& { return *current_entity->scene; }

auto CppScript::get_scene() -> Scene& { return *current_entity->scene; }

auto CppScript::transform() -> Components::Transform& { return get_entity().get_components<Components::Transform>(); }

auto CppScript::transform() const -> const Components::Transform& { return get_entity().get_components<Components::Transform>(); }

CppScript::CppScript(const Collections::StringViewMap<Parameter>& params)
	: parameters(params)
{
	current_entity = make_scope<Entity, PimplDeleter<Entity>>();
}

} // namespace Disarray
