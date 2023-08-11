#pragma once

#include "scene/Component.hpp"
#include "scene/Entity.hpp"

#include <tuple>
#include <yaml-cpp/yaml.h>

namespace Disarray {

	template <ValidComponent T, class Child> struct ComponentSerialiser {
		void serialise(Entity& entity, const T& component) { static_cast<Child&>(*this).serialise_impl(entity, component); };
	};

	template <class... Serialisers> struct Serialiser {
		std::tuple<Serialisers...> serialisers {};
	};

} // namespace Disarray
