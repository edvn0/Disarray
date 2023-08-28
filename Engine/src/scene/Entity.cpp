#include "DisarrayPCH.hpp"

#include "scene/Entity.hpp"

#include <fmt/format.h>

#include "core/Log.hpp"
#include "scene/Components.hpp"
#include "scene/Scene.hpp"
#include "util/FormattingUtilities.hpp"

auto fmt::formatter<entt::entity>::format(entt::entity c, fmt::format_context& ctx) const -> decltype(ctx.out())
{
	auto as_uint = entt::to_integral(c);
	return formatter<string_view>::format(fmt::format("{}", as_uint), ctx);
}

auto fmt::formatter<Disarray::Entity>::format(const Disarray::Entity& c, fmt::format_context& ctx) const -> decltype(ctx.out())
{
	auto as_uint = entt::to_integral(c.get_identifier());
	return formatter<string_view>::format(fmt::format("{}", as_uint), ctx);
}

namespace Disarray {

// TODO: ID will need some UUID, for now, lets just increase by one
static Identifier global_identifier { 1 };

Entity::Entity(Scene* input_scene, std::string_view input_name)
	: scene(input_scene)
	, name(input_name)
	, identifier(scene->get_registry().create())
{
	try_add_component<Components::Transform>();
	try_add_component<Components::ID>(global_identifier++);
	try_add_component<Components::Tag>(name);
}

Entity::Entity(Scene* scene, entt::entity handle)
	: Entity(scene, handle, "Empty")
{
}

Entity::Entity(Scene* scene)
	: Entity(scene, entt::null, "Empty")
{
}

Entity::Entity(Scene* s, entt::entity entity, std::string_view n)
	: scene(s)
	, name(n)
	, identifier(entity)
{
}

ImmutableEntity::ImmutableEntity(const Scene* scene, entt::entity handle)
	: ImmutableEntity(scene, handle, "Empty")
{
}

ImmutableEntity::ImmutableEntity(const Scene* scene)
	: ImmutableEntity(scene, entt::null, "Empty")
{
}

ImmutableEntity::ImmutableEntity(const Scene* input_scene, entt::entity entity, std::string_view input_name)
	: scene(input_scene)
	, name(input_name)
	, identifier(entity)
{
}
auto ImmutableEntity::get_registry() const -> const entt::registry& { return scene->get_registry(); }
auto Entity::get_registry() -> entt::registry& { return scene->get_registry(); }
auto Entity::get_registry() const -> const entt::registry& { return scene->get_registry(); }

Entity Entity::deserialise(Disarray::Scene& scene, entt::entity handle, Disarray::Identifier entity_id, std::string_view name)
{
	Entity entity { &scene, handle, name };
	entity.get_components<Components::ID>().identifier = entity_id;
	return entity;
}

void Entity::add_child(Entity& child)
{
	if (!has_component<Components::Inheritance>()) {
		add_component<Components::Inheritance>();
	}

	auto& inheritance_info = get_components<Components::Inheritance>();
	inheritance_info.add_child(child);

	if (!child.has_component<Components::Inheritance>()) {
		auto& child_inheritance = child.add_component<Components::Inheritance>();
		child_inheritance.parent = get_components<Components::ID>().identifier;
	}
}

void Entity::add_child(Entity* child_of_this)
{
	if (!child_of_this) {
		Log::debug("Entity add_child", "Associated child was nullptr.");
		return;
	}

	add_child(*child_of_this);
}

} // namespace Disarray
