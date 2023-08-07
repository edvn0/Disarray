#include "DisarrayPCH.hpp"

#include "scene/Entity.hpp"

#include "core/Log.hpp"
#include "scene/Components.hpp"
#include "util/FormattingUtilities.hpp"

namespace Disarray {

	Entity::Entity(Scene& s, std::string_view n)
		: scene(s)
		, name(n)
	{
	}

	Entity::Entity(Scene& s, entt::entity entity, std::string_view n)
		: scene(s)
		, name(n)
		, identifier(entity)
	{
	}

	void Entity::add_child(Entity& child)
	{
		if (!has_component<Inheritance>()) {
			add_component<Inheritance>();
		}

		auto& inheritance_info = get_components<Inheritance>();
		inheritance_info.child = child.get_identifier();
		inheritance_info.parent = get_identifier();
		Log::debug("Entity",
			fmt::format(
				"Mapped parent: {} to child: {}", static_cast<std::uint32_t>(get_identifier()), static_cast<std::uint32_t>(child.get_identifier())));
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
