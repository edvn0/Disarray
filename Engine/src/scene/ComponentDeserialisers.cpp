#include "core/Collections.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantLayout.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Scripts.hpp"
#include "scene/SerialisationTypeConversions.hpp"

namespace Disarray {

using json = nlohmann::json;
using namespace std::string_view_literals;

/*
auto PipelineDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("properties"); }
void PipelineDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Pipeline& pipeline, const Device& device)
{
	auto props = object["properties"];
	PipelineProperties properties {
		.vertex_shader = Shader::compile(device, props["vertex_shader"].get<std::filesystem::path>()),
		.fragment_shader = Shader::compile(device, props["fragment_shader"].get<std::filesystem::path>()),
		// framebuffer,
		.layout = [](const json& vertex_layout) -> VertexLayout {
			VertexLayout layout {};
			const auto& binding = vertex_layout["binding"];
			layout.binding = {
				.binding = binding["binding"],
				.stride = binding["stride"],
				.input_rate = to_enum_value<InputRate>(binding, "input_rate").value_or(InputRate::Vertex),
			};
			layout.total_size = vertex_layout["total_size"];

			const auto& array = vertex_layout["elements"];
			ensure(array.is_array());
			for (const auto& json_layout : array) {
				auto& element = layout.elements.emplace_back(*to_enum_value<ElementType>(json_layout, "type"), json_layout["debug_name"]);
				element.offset = json_layout["offset"];
				element.size = json_layout["size"];
			}
			return layout;
		}(props["vertex_layout"]),
		.push_constant_layout = [](const json& push_constant_layout) -> PushConstantLayout {
			std::vector<PushConstantRange> layout {};
			layout.reserve(push_constant_layout["size"]);

			for (const auto& flag_size_offset_object : push_constant_layout["ranges"]) {
				PushConstantRange& added = layout.emplace_back();
				added.size = flag_size_offset_object["size"];
				added.offset = flag_size_offset_object["offset"];
				added.flags = flag_size_offset_object["flags"];
			}
			return PushConstantLayout { std::move(layout) };
		}(props["push_constant_layout"]),
		.extent = props["extent"],
		.polygon_mode = to_enum_value<PolygonMode>(props, "polygon_mode").value_or(PolygonMode::Fill),
		.line_width = props["line_width"],
		.samples = to_enum_value<SampleCount>(props, "samples").value_or(SampleCount::One),
		.depth_comparison_operator
		= to_enum_value<DepthCompareOperator>(props, "depth_comparison_operator").value_or(DepthCompareOperator::LessOrEqual),
		.cull_mode = to_enum_value<CullMode>(props, "cull_mode").value_or(CullMode::Back),
		.face_mode = to_enum_value<FaceMode>(props, "face_mode").value_or(FaceMode::CounterClockwise),
		.write_depth = props["write_depth"],
		.test_depth = props["test_depth"],
		.descriptor_set_layouts = {},
	};

	pipeline.pipeline = Pipeline::construct(device, properties);
}
 */

auto ScriptDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("identifier"); }
void ScriptDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Script& script, const Device& device)
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

	auto props = object["properties"];
	MeshProperties properties {
		.path = props["path"],
		.initial_rotation = props["initial_rotation"],
	};

	mesh.mesh = Mesh::construct(device, properties);
}

auto TextureDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("properties"); }
void TextureDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Texture& texture, const Device& device)
{
	auto props = object["properties"];
	TextureProperties properties {
		.extent = props["extent"],
		.format = to_enum_value<ImageFormat>(props, "format").value_or(ImageFormat::SBGR),
		.mips = props["mips"],
		.path = props["path"],
		.debug_name = props["debug_name"],
	};
	texture.texture = Texture::construct(device, properties);
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

auto TransformDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool { return true; }
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
	return object_for_the_component.contains("factor");
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

auto PointLightDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool { return true; }
void PointLightDeserialiser::deserialise_impl(const nlohmann::json& object, Components::PointLight& light, const Device& /*unused*/)
{
	light.ambient = object["ambient"];
	light.diffuse = object["diffuse"];
	light.specular = object["specular"];
	light.factors = object["factors"];
}

} // namespace Disarray

NLOHMANN_JSON_NAMESPACE_BEGIN
void adl_serializer<Disarray::Parameter>::to_json(json& object, const Disarray::Parameter& param) { }
void adl_serializer<Disarray::Parameter>::from_json(const json& object, Disarray::Parameter& param) { }
NLOHMANN_JSON_NAMESPACE_END
