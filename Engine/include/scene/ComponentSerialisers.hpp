#pragma once

#include "scene/Component.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

namespace Disarray {

	enum class SerialiserType : std::uint8_t { Faulty, Pipeline, Texture, Mesh, Transform, LineGeometry, QuadGeometry, Inheritance };
	template <class T> constexpr SerialiserType serialiser_type_for = SerialiserType::Faulty;

	template <ValidComponent T, class Child> struct ComponentSerialiser {
		bool can_serialise(const Entity& entity) { return entity.has_component<T>(); }
		constexpr std::string_view get_component_name() { return magic_enum::enum_name(serialiser_type_for<T>); }
		void serialise(const T& component, nlohmann::json& object_for_the_component)
		{
			static_cast<Child&>(*this).serialise_impl(component, object_for_the_component);
		};
	};

	namespace {
#define MAKE_SERIALISER(Name, ComponentType)                                                                                                         \
	struct Name : public ComponentSerialiser<Components::ComponentType, Name> {                                                                      \
		static constexpr SerialiserType type = SerialiserType::ComponentType;                                                                        \
		void serialise_impl(const Components::ComponentType&, nlohmann::json&);                                                                      \
	};                                                                                                                                               \
	template <> constexpr SerialiserType serialiser_type_for<Components::ComponentType> = SerialiserType::ComponentType;
	} // namespace

	MAKE_SERIALISER(PipelineSerialiser, Pipeline)
	MAKE_SERIALISER(MeshSerialiser, Mesh)
	MAKE_SERIALISER(TextureSerialiser, Texture)
	MAKE_SERIALISER(TransformSerialiser, Transform)
	MAKE_SERIALISER(LineGeometrySerialiser, LineGeometry)
	MAKE_SERIALISER(QuadGeometrySerialiser, QuadGeometry)
	MAKE_SERIALISER(InheritanceSerialiser, Inheritance)

} // namespace Disarray
