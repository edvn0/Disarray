#include "DisarrayPCH.hpp"

#include "core/Log.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"
#include "util/FormattingUtilities.hpp"

#include <fmt/format.h>

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

Entity::Entity(Scene& s, std::string_view n)
	: scene(s)
	, name(n)
	, identifier(scene.get_registry().create())
{
	try_add_component<Components::Transform>();
	try_add_component<Components::ID>(global_identifier++);
	try_add_component<Components::Tag>(name);
}

Entity::Entity(Scene& scene, entt::entity handle)
	: Entity(scene, handle, "Empty")
{
}

Entity::Entity(Scene& scene)
	: Entity(scene, entt::null, "Empty")
{
}

Entity::Entity(Scene& s, entt::entity entity, std::string_view n)
	: scene(s)
	, name(n)
	, identifier(entity)
{
}

Entity Entity::deserialise(Disarray::Scene& scene, entt::entity handle, Disarray::Identifier id, std::string_view name)
{
	Entity entity { scene, name };
	entity.get_components<Components::ID>().identifier = id;
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
