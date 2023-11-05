#include "DisarrayPCH.hpp"

#include <optional>

#include "scene/ComponentSerialisers.hpp"
#include "scene/Components.hpp"
#include "scene/CppScript.hpp"
#include "scene/SerialisationTypeConversions.hpp"

namespace Disarray {

using json = nlohmann::json;
using namespace std::string_view_literals;

template <class T> static constexpr auto assign_or_noop(auto& json_object, std::string_view key, const T& value)
{
	if (json_object.contains(key)) {
		return;
	}
	json_object[key] = value;
}

template <class T> static constexpr auto assign_or_noop(auto& json_object, std::string_view key, const std::optional<T>& value)
{
	if (!value.has_value()) {
		return;
	}
	assign_or_noop<T>(json_object, key, *value);
}

void ScriptSerialiser::serialise_impl(const Components::Script& script, nlohmann::json& object)
{
	const auto& cpp_script = script.get_script();
	const auto& identifier = cpp_script.identifier();
	object["identifier"] = identifier;
}

void MeshSerialiser::serialise_impl(const Components::Mesh& mesh, nlohmann::json& object)
{
	if (mesh.mesh) {
		json properties;
		const auto& props = mesh.mesh->get_properties();
		properties["path"] = props.path;
		properties["initial_rotation"] = props.initial_rotation;
		object["properties"] = properties;
	}
}

void SkyboxSerialiser::serialise_impl(const Components::Skybox& skybox, nlohmann::json& object)
{
	object["colour"] = skybox.colour;
	if (skybox.texture) {
		object["texture_path"] = skybox.texture->get_properties().path;
	}
}

void TextSerialiser::serialise_impl(const Components::Text& text, nlohmann::json& object)
{
	object["text_data"] = text.text_data;
	object["colour"] = text.colour;
	object["size"] = text.size;
	object["projection"] = magic_enum::enum_name(text.projection);
}

void PillColliderSerialiser::serialise_impl(const Components::PillCollider& pill, nlohmann::json& object) { object["radius"] = pill.radius; }

void SphereColliderSerialiser::serialise_impl(const Components::SphereCollider& sphere, nlohmann::json& object) { object["radius"] = sphere.radius; }

void BoxColliderSerialiser::serialise_impl(const Components::BoxCollider& box, nlohmann::json& object) { object["half_size"] = box.half_size; }

void TextureSerialiser::serialise_impl(const Components::Texture& texture, nlohmann::json& object)
{
	object["colour"] = texture.colour;

	if (texture.texture) {
		const TextureProperties& props = texture.texture->get_properties();
		json properties;
		properties["path"] = props.path;
		properties["extent"] = props.extent;
		properties["format"] = magic_enum::enum_name(props.format);
		properties["mips"] = props.mips ? *props.mips : 1;
		properties["path"] = props.path;
		properties["debug_name"] = props.debug_name;
		object["properties"] = properties;
	}
}

void InheritanceSerialiser::serialise_impl(const Components::Inheritance& inheritance, nlohmann::json& object)
{
	if (!inheritance.children.empty()) {
		object["children"] = inheritance.children;
	}
	if (inheritance.parent > 0) {
		object["parent"] = inheritance.parent;
	}
}

void TransformSerialiser::serialise_impl(const Components::Transform& transform, nlohmann::json& object)
{
	object["rotation"] = transform.rotation;
	object["position"] = transform.position;
	object["scale"] = transform.scale;
}

void DirectionalLightSerialiser::serialise_impl(const Components::DirectionalLight& light, nlohmann::json& object)
{
	const auto& params = light.projection_parameters;
	object["projection_parameters"] = {
		{
			"factor",
			params.factor,
		},
		{
			"near",
			params.near,
		},
		{
			"far",
			params.far,
		},
		{
			"fov",
			params.fov,
		},
	};
	object["direction"] = light.direction;
	object["ambient"] = light.ambient;
	object["diffuse"] = light.diffuse;
	object["specular"] = light.specular;

	object["use_direction_vector"] = light.use_direction_vector;
}

void PointLightSerialiser::serialise_impl(const Components::PointLight& light, nlohmann::json& object)
{
	object["factors"] = light.factors;
	object["ambient"] = light.ambient;
	object["diffuse"] = light.diffuse;
	object["specular"] = light.specular;
}

void LineGeometrySerialiser::serialise_impl(const Components::LineGeometry& geom, nlohmann::json& object)
{
	object["to_position"] = geom.to_position;
	object["geometry"] = magic_enum::enum_name(geom.geometry);
}

void QuadGeometrySerialiser::serialise_impl(const Components::QuadGeometry& geom, nlohmann::json& object)
{
	object["geometry"] = magic_enum::enum_name(geom.geometry);
}

} // namespace Disarray
