#pragma once

#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "core/Log.hpp"
#include "core/exceptions/BaseException.hpp"
#include "scene/Component.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

namespace Disarray {

class ComponentDeserialiseException : public BaseException {
public:
	explicit ComponentDeserialiseException(std::string_view scope, std::string_view message)
		: BaseException(scope, message)
	{
	}
};

enum class SerialiserType : std::uint8_t {
	Faulty,
	Tag,
	Transform,
	ID,
	Inheritance,
	LineGeometry,
	QuadGeometry,
	Mesh,
	Material,
	Texture,
	DirectionalLight,
	PointLight,
	SpotLight,
	Script,
	Controller,
	Camera,
	RigidBody,
	BoxCollider,
	SphereCollider,
	CapsuleCollider,
	ColliderMaterial,
	Skybox,
	Text
};
template <class T> inline constexpr SerialiserType serialiser_type_for = SerialiserType::Faulty;

template <ValidComponent T, class Child> struct ComponentSerialiser {
	auto can_serialise(const ImmutableEntity& entity) -> bool { return entity.has_component<T>(); }

	constexpr auto get_component_name() -> std::string_view { return magic_enum::enum_name(serialiser_type_for<T>); }
	void serialise(const T& component, nlohmann::json& object_for_the_component)
	{
		static_cast<Child&>(*this).serialise_impl(component, object_for_the_component);
	};
};

template <ValidComponent T, class Child> struct ComponentDeserialiser {
	auto can_serialise(const Entity& entity) -> bool { return entity.has_component<T>(); }
	constexpr auto get_component_name() -> std::string_view { return magic_enum::enum_name(serialiser_type_for<T>); }

	auto should_add_component(const nlohmann::json& object_for_the_component) -> bool
	{
		return static_cast<Child&>(*this).should_add_component_impl(object_for_the_component);
	};
	void deserialise(const nlohmann::json& object_for_the_component, T& component, const Device& device)
	{
		static_cast<Child&>(*this).deserialise_impl(object_for_the_component, component, device);
	};
};

#define MAKE_SERIALISER(Name, ComponentType)                                                                                                         \
	struct Name : public ComponentSerialiser<Components::ComponentType, Name> {                                                                      \
		static constexpr SerialiserType type = SerialiserType::ComponentType;                                                                        \
		void serialise_impl(const Components::ComponentType&, nlohmann::json&);                                                                      \
	};                                                                                                                                               \
	template <> inline constexpr SerialiserType serialiser_type_for<Components::ComponentType> = SerialiserType::ComponentType;

#define MAKE_DESERIALISER(Name, ComponentType)                                                                                                       \
	struct Name : public ComponentDeserialiser<Components::ComponentType, Name> {                                                                    \
		static constexpr SerialiserType type = SerialiserType::ComponentType;                                                                        \
		void deserialise_impl(const nlohmann::json&, Components::ComponentType&, const Device&);                                                     \
		auto should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool;                                                      \
	};

MAKE_SERIALISER(ScriptSerialiser, Script)
MAKE_SERIALISER(MeshSerialiser, Mesh)
MAKE_SERIALISER(SkyboxSerialiser, Skybox)
MAKE_SERIALISER(TextSerialiser, Text)
MAKE_SERIALISER(BoxColliderSerialiser, BoxCollider)
MAKE_SERIALISER(SphereColliderSerialiser, SphereCollider)
MAKE_SERIALISER(CapsuleColliderSerialiser, CapsuleCollider)
MAKE_SERIALISER(ColliderMaterialSerialiser, ColliderMaterial)
MAKE_SERIALISER(RigidBodySerialiser, RigidBody)
MAKE_SERIALISER(TextureSerialiser, Texture)
MAKE_SERIALISER(TransformSerialiser, Transform)
MAKE_SERIALISER(LineGeometrySerialiser, LineGeometry)
MAKE_SERIALISER(QuadGeometrySerialiser, QuadGeometry)
MAKE_SERIALISER(DirectionalLightSerialiser, DirectionalLight)
MAKE_SERIALISER(PointLightSerialiser, PointLight)
MAKE_SERIALISER(SpotLightSerialiser, SpotLight)
MAKE_SERIALISER(InheritanceSerialiser, Inheritance)

MAKE_DESERIALISER(ScriptDeserialiser, Script)
MAKE_DESERIALISER(MeshDeserialiser, Mesh)
MAKE_DESERIALISER(SkyboxDeserialiser, Skybox)
MAKE_DESERIALISER(TextDeserialiser, Text)
MAKE_DESERIALISER(BoxColliderDeserialiser, BoxCollider)
MAKE_DESERIALISER(SphereColliderDeserialiser, SphereCollider)
MAKE_DESERIALISER(CapsuleColliderDeserialiser, CapsuleCollider)
MAKE_DESERIALISER(ColliderMaterialDeserialiser, ColliderMaterial)
MAKE_DESERIALISER(RigidBodyDeserialiser, RigidBody)
MAKE_DESERIALISER(TextureDeserialiser, Texture)
MAKE_DESERIALISER(TransformDeserialiser, Transform)
MAKE_DESERIALISER(LineGeometryDeserialiser, LineGeometry)
MAKE_DESERIALISER(QuadGeometryDeserialiser, QuadGeometry)
MAKE_DESERIALISER(DirectionalLightDeserialiser, DirectionalLight)
MAKE_DESERIALISER(PointLightDeserialiser, PointLight)
MAKE_DESERIALISER(SpotLightDeserialiser, SpotLight)
MAKE_DESERIALISER(InheritanceDeserialiser, Inheritance)

} // namespace Disarray
