#pragma once

#include <type_traits>

#include "core/Concepts.hpp"
#include "scene/Components.hpp"

namespace Disarray {

// Disallow empty structs, or structs that contain references instead of IRCs
namespace Detail {
	template <class T>
	concept IsInAllowedComponents = AnyOf<T, Components::Tag, Components::Transform, Components::ID, Components::Inheritance,
		Components::LineGeometry, Components::QuadGeometry, Components::Mesh, Components::Material, Components::Texture, Components::DirectionalLight,
		Components::PointLight, Components::SpotLight, Components::Script, Components::Controller, Components::Camera, Components::BoxCollider,
		Components::SphereCollider, Components::CapsuleCollider, Components::ColliderMaterial, Components::RigidBody, Components::Skybox,
		Components::Text>;

	template <typename... Component> struct ComponentGroup { };
} // namespace Detail

using AllComponents
	= Detail::ComponentGroup<Components::Tag, Components::Transform, Components::ID, Components::Inheritance, Components::LineGeometry,
		Components::QuadGeometry, Components::Mesh, Components::Material, Components::Texture, Components::DirectionalLight, Components::PointLight,
		Components::SpotLight, Components::Script, Components::Controller, Components::Camera, Components::BoxCollider, Components::SphereCollider,
		Components::CapsuleCollider, Components::ColliderMaterial, Components::RigidBody, Components::Skybox, Components::Text>;

using NonDeletableComponents = Detail::ComponentGroup<Components::Tag, Components::Transform, Components::ID>;

template <class T>
concept ValidComponent = !std::is_empty_v<T>
	&& ((Detail::IsInAllowedComponents<T> && std::is_default_constructible_v<T> && std::is_copy_constructible_v<T>)
		|| (Detail::IsInAllowedComponents<std::remove_const_t<T>> && std::is_default_constructible_v<std::remove_const_t<T>>));

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
