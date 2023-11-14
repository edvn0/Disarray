#include "core/Collections.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "physics/PhysicsProperties.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Components.hpp"
#include "scene/Scripts.hpp"
#include "scene/SerialisationTypeConversions.hpp"

namespace Disarray {

using json = nlohmann::json;
using namespace std::string_view_literals;

auto SkyboxDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void SkyboxDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Skybox& skybox, const Device& device)
{
	skybox.colour = {};
	if (object.contains("colour")) {
		skybox.colour = object["colour"];
	}
	if (object.contains("texture_path")) {
		skybox.texture = Texture::construct(device,
			{
				.path = object["texture_path"],
				.dimension = TextureDimension::Three,
				.debug_name = object["texture_path"],
			});
	}
}

auto TextDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void TextDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Text& text, const Device&)
{
	text.text_data = object["text_data"];
	text.colour = object["colour"];
	text.size = object["size"];
	text.projection = to_enum_value<Components::TextProjection>(object, "projection").value_or(Components::TextProjection::WorldSpace);
}

auto CapsuleColliderDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void CapsuleColliderDeserialiser::deserialise_impl(const nlohmann::json& object, Components::CapsuleCollider& pill, const Device&)
{
	pill.radius = object["radius"];
	pill.height = object["height"];
	pill.offset = object["offset"];
}

auto ColliderMaterialDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void ColliderMaterialDeserialiser::deserialise_impl(const nlohmann::json& object, Components::ColliderMaterial& material, const Device&)
{
	material.bounciness = object["bounciness"];
	material.friction_coefficient = object["friction_coefficient"];
	material.mass_density = object["mass_density"];
}

auto BoxColliderDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void BoxColliderDeserialiser::deserialise_impl(const nlohmann::json& object, Components::BoxCollider& box, const Device&)
{
	box.half_size = object["half_size"];
	box.offset = object["offset"];
}

auto SphereColliderDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void SphereColliderDeserialiser::deserialise_impl(const nlohmann::json& object, Components::SphereCollider& sphere, const Device&)
{
	sphere.radius = object["radius"];
	sphere.offset = object["offset"];
}

auto RigidBodyDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void RigidBodyDeserialiser::deserialise_impl(const nlohmann::json& object, Components::RigidBody& rigid_body, const Device&)
{
	rigid_body.body_type = to_enum_value<BodyType>(object, "body_type").value_or(BodyType::Static);
	rigid_body.mass = object["mass"];
	rigid_body.linear_drag = object["linear_drag"];
	rigid_body.angular_drag = object["angular_drag"];
	rigid_body.disable_gravity = object["disable_gravity"];
	rigid_body.is_kinematic = object["is_kinematic"];
}

auto ScriptDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("identifier"); }
void ScriptDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Script& script, const Device&)
{
	const auto& identifier = object["identifier"];
	Collections::StringViewMap<Parameter> parameters {};

	auto json_parameters = std::optional<json> {};
	if (object.contains("parameters")) {
		json_parameters = object["parameters"];
	}

	if (json_parameters.has_value()) {
		for (const auto& [k, v] : json_parameters->items()) {
			parameters[k] = v;
			Log::info("ComponentDeserialiser", "parameters[{}] = {}", k, v.dump());
		}
	}

	if (identifier == "MoveInCircle") {
		script.bind<Scripts::MoveInCircleScript>(parameters);
	}
	if (identifier == "LinearMovement") {
		script.bind<Scripts::LinearMovementScript>(parameters);
	}
}

auto MeshDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("properties"); }
void MeshDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Mesh& mesh, const Device& device)
{

	const auto& props = object["properties"];
	MeshProperties properties {
		.path = props["path"],
		.initial_rotation = props["initial_rotation"],
	};

	mesh.mesh = Mesh::construct(device, properties);
}

auto TextureDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void TextureDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Texture& texture, const Device& device)
{
	if (object.contains("properties")) {
		const auto& props = object["properties"];
		TextureProperties properties {
			.extent = props["extent"],
			.format = to_enum_value<ImageFormat>(props, "format").value_or(ImageFormat::SBGR),
			.mips = props["mips"],
			.path = props["path"],
			.debug_name = props["debug_name"],
		};
		texture.texture = Texture::construct(device, properties);
	}
	texture.colour = object["colour"];
}

auto InheritanceDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool
{
	return object.contains("children") || object.contains("parent");
}
void InheritanceDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Inheritance& inheritance, const Device& /*unused*/)
{
	if (object.contains("children")) {
		std::unordered_set<Identifier> children;
		object["children"].get_to(children);
		inheritance.children = std::move(children);
	}

	if (object.contains("parent")) {
		inheritance.parent = object["parent"];
	}
}

auto TransformDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void TransformDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Transform& transform, const Device& /*unused*/)
{
	transform.rotation = object["rotation"];
	transform.position = object["position"];
	transform.scale = object["scale"];
}

auto LineGeometryDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool
{
	return object_for_the_component.contains("to_position") && object_for_the_component.contains("geometry");
}
void LineGeometryDeserialiser::deserialise_impl(const nlohmann::json& object, Components::LineGeometry& geom, const Device& /*unused*/)
{
	auto enum_val = to_enum_value<Geometry>(object, "geometry"sv);
	geom.geometry = enum_val.value_or(Geometry::Line);
	geom.to_position = object["to_position"];
}

auto QuadGeometryDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool
{
	return object_for_the_component.contains("geometry");
}
void QuadGeometryDeserialiser::deserialise_impl(const nlohmann::json& object, Components::QuadGeometry& geom, const Device& /*unused*/)
{
	auto enum_val = to_enum_value<Geometry>(object, "geometry"sv);
	geom.geometry = enum_val.value_or(Geometry::Rectangle);
}

auto DirectionalLightDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool
{
	return object_for_the_component.contains("projection_parameters");
}
void DirectionalLightDeserialiser::deserialise_impl(const nlohmann::json& object, Components::DirectionalLight& light, const Device& /*unused*/)
{
	const auto& params = object["projection_parameters"];
	light.projection_parameters.factor = params["factor"];
	light.projection_parameters.near = params["near"];
	light.projection_parameters.far = params["far"];
	light.projection_parameters.fov = params["fov"];

	light.direction = object["direction"];
	light.ambient = object["ambient"];
	light.diffuse = object["diffuse"];
	light.specular = object["specular"];

	light.use_direction_vector = object["use_direction_vector"];
}

auto PointLightDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void PointLightDeserialiser::deserialise_impl(const nlohmann::json& object, Components::PointLight& light, const Device& /*unused*/)
{
	light.ambient = object["ambient"];
	light.diffuse = object["diffuse"];
	light.specular = object["specular"];
	light.factors = object["factors"];
}

auto SpotLightDeserialiser::should_add_component_impl(const nlohmann::json&) -> bool { return true; }
void SpotLightDeserialiser::deserialise_impl(const nlohmann::json& object, Components::SpotLight& light, const Device& /*unused*/)
{
	light.direction = object["direction"];
	light.factors = object["factors"];
	light.cutoff_angle_degrees = object["cutoff_angle_degrees"];
	if (object.contains("outer_cutoff_angle_degrees")) {
		light.outer_cutoff_angle_degrees = object["outer_cutoff_angle_degrees"];
	}
	light.ambient = object["ambient"];
	light.diffuse = object["diffuse"];
	light.specular = object["specular"];
}

} // namespace Disarray

NLOHMANN_JSON_NAMESPACE_BEGIN
void adl_serializer<Disarray::Parameter>::to_json(json& object, const Disarray::Parameter& param) { }
void adl_serializer<Disarray::Parameter>::from_json(const json& object, Disarray::Parameter& param) { }
NLOHMANN_JSON_NAMESPACE_END
