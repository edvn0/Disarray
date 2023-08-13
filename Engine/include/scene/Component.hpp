#pragma once

#include "core/Concepts.hpp"
#include "scene/Components.hpp"

#include <type_traits>

namespace Disarray {

	// Disallow empty structs, or structs that contain references instead of IRCs
	namespace {
		template <class T>
		concept IsInAllowedComponents = AnyOf<T, Components::Tag, Components::Transform, Components::ID, Components::Inheritance,
			Components::LineGeometry, Components::QuadGeometry, Components::Mesh, Components::Material, Components::Pipeline, Components::Texture>;
	}

	template <class T>
	concept ValidComponent = (IsInAllowedComponents<T> && std::is_default_constructible_v<T> && std::is_copy_constructible_v<T>);

	template <class T>
	concept DeletableComponent = ValidComponent<T> && (!std::is_same_v<T, Components::ID> && !std::is_same_v<T, Components::Tag>);

} // namespace Disarray
