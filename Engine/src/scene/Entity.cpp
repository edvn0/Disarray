#include "DisarrayPCH.hpp"

#include "scene/Entity.hpp"

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

	void Entity::add_child(Entity child_of_this) { }

} // namespace Disarray
