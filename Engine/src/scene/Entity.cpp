#include "DisarrayPCH.hpp"

#include "scene/Entity.hpp"

#include "core/Log.hpp"
#include "scene/Components.hpp"
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
	static Identifier global_identifier { 0 };

	Entity::Entity(Scene& s, std::string_view n)
		: scene(s)
		, name(n)
	{
		add_component<Transform>();
		add_component<ID>(global_identifier++);
		add_component<Tag>(name);
	}

	Entity::Entity(Scene& s, entt::entity entity, std::string_view n)
		: scene(s)
		, name(n)
		, identifier(entity)
	{
		add_component<Transform>();
		add_component<ID>(global_identifier++);
		add_component<Tag>(name);
	}

	void Entity::add_child(Entity& child)
	{
		if (!has_component<Inheritance>()) {
			add_component<Inheritance>();
		}

		auto& inheritance_info = get_components<Inheritance>();
		inheritance_info.add_child(child);
		inheritance_info.parent = get_identifier();
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
