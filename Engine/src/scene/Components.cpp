#include "DisarrayPCH.hpp"

#include "scene/Components.hpp"

#include "scene/Entity.hpp"

namespace Disarray {

	void Inheritance::add_child(Entity& entity) { children.insert(entity.get_identifier()); }

} // namespace Disarray
