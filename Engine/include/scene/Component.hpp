#pragma once

#include <type_traits>

#include "core/Concepts.hpp"
#include "scene/Components.hpp"

namespace Disarray {

// Disallow empty structs, or structs that contain references instead of IRCs
namespace {
	template <class T>
	concept IsInAllowedComponents = AnyOf<T, Components::Tag, Components::Transform, Components::ID, Components::Inheritance,
		Components::LineGeometry, Components::QuadGeometry, Components::Mesh, Components::Material, Components::Pipeline, Components::Texture,
		Components::DirectionalLight, Components::PointLight, Components::Script>;
} // namespace

template <class T>
concept ValidComponent = (IsInAllowedComponents<T> && std::is_default_constructible_v<T>)
	|| (IsInAllowedComponents<std::remove_const_t<T>> && std::is_default_constructible_v<std::remove_const_t<T>>);

template <class T>
concept DeletableComponent
	= ValidComponent<T> && (!std::is_same_v<std::remove_const<T>, Components::ID> && !std::is_same_v<std::remove_const<T>, Components::Tag>);

template <ValidComponent T> class MissingComponentException : public BaseException {
public:
	explicit MissingComponentException()
		: BaseException("MissingComponentException", fmt::format("Missing component {}", Components::component_name<T>))
	{
	}
};

} // namespace Disarray
