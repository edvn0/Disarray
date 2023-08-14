#pragma once

#include "scene/Component.hpp"
#include "scene/Components.hpp"
#include "scene/Entity.hpp"

#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

namespace Disarray {

class ComponentDeserialiseException : public std::runtime_error {
public:
	explicit ComponentDeserialiseException(const std::string& message)
		: runtime_error(message)
	{
		Log::error("ComponentDeserialiseException", "{}", message);
	}
	~ComponentDeserialiseException() noexcept override = default;
};

enum class SerialiserType : std::uint8_t { Faulty, Pipeline, Texture, Mesh, Transform, LineGeometry, QuadGeometry, Inheritance };
template <class T> inline constexpr SerialiserType serialiser_type_for = SerialiserType::Faulty;

template <ValidComponent T, class Child> struct ComponentSerialiser {
	bool can_serialise(const Entity& entity) { return entity.has_component<T>(); }

	constexpr std::string_view get_component_name() { return magic_enum::enum_name(serialiser_type_for<T>); }
	void serialise(const T& component, nlohmann::json& object_for_the_component)
	{
		static_cast<Child&>(*this).serialise_impl(component, object_for_the_component);
	};
};

template <ValidComponent T, class Child> struct ComponentDeserialiser {
	bool can_serialise(const Entity& entity) { return entity.has_component<T>(); }
	constexpr std::string_view get_component_name() { return magic_enum::enum_name(serialiser_type_for<T>); }

	bool should_add_component(const nlohmann::json& object_for_the_component)
	{
		return static_cast<Child&>(*this).should_add_component_impl(object_for_the_component);
	};
	void deserialise(const nlohmann::json& object_for_the_component, T& component, const Device& device)
	{
		static_cast<Child&>(*this).deserialise_impl(object_for_the_component, component, device);
	};
};

namespace {
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
		bool should_add_component_impl(const nlohmann::json& object_for_the_component);                                                              \
	};

} // namespace

MAKE_SERIALISER(PipelineSerialiser, Pipeline)
MAKE_SERIALISER(MeshSerialiser, Mesh)
MAKE_SERIALISER(TextureSerialiser, Texture)
MAKE_SERIALISER(TransformSerialiser, Transform)
MAKE_SERIALISER(LineGeometrySerialiser, LineGeometry)
MAKE_SERIALISER(QuadGeometrySerialiser, QuadGeometry)
MAKE_SERIALISER(InheritanceSerialiser, Inheritance)

MAKE_DESERIALISER(PipelineDeserialiser, Pipeline)
MAKE_DESERIALISER(MeshDeserialiser, Mesh)
MAKE_DESERIALISER(TextureDeserialiser, Texture)
MAKE_DESERIALISER(TransformDeserialiser, Transform)
MAKE_DESERIALISER(LineGeometryDeserialiser, LineGeometry)
MAKE_DESERIALISER(QuadGeometryDeserialiser, QuadGeometry)
MAKE_DESERIALISER(InheritanceDeserialiser, Inheritance)

} // namespace Disarray
