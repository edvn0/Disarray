#pragma once

#include "core/Concepts.hpp"
#include "scene/Components.hpp"

#include <type_traits>

namespace Disarray {

	// Disallow empty structs, or structs that contain references instead of IRCs
	namespace {
		template <class T>
		concept IsInAllowedComponents
			= AnyOf<T, Tag, Transform, ID, Inheritance, Components::Mesh, Components::Pipeline, Components::Texture, Components::Geometry>;
	}

	template <class T>
	concept ValidComponent = (IsInAllowedComponents<T> && sizeof(T) > 1 && std::is_default_constructible_v<T> && std::is_copy_constructible_v<T>);

} // namespace Disarray
